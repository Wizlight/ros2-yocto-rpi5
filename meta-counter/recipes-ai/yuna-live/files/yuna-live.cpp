#include <net.h>
#include <mat.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>

namespace fs = std::filesystem;

static const int SRC_W = 800;
static const int SRC_H = 600;
static const size_t FRAME_SIZE = SRC_W * SRC_H * 4;
static const int PORT = 5000;

bool send_all(int sock, const void* data, size_t size)
{
    const char* p = static_cast<const char*>(data);

    while (size > 0) {
        ssize_t sent = send(sock, p, size, MSG_NOSIGNAL);

        if (sent <= 0)
            return false;

        p += sent;
        size -= sent;
    }

    return true;
}

std::string find_latest_frame()
{
    std::string latest;

    if (!fs::exists("/tmp/livecam"))
        return latest;

    for (const auto& entry : fs::directory_iterator("/tmp/livecam")) {
        if (!entry.is_regular_file())
            continue;

        if (entry.file_size() != FRAME_SIZE)
            continue;

        std::string path = entry.path().string();

        if (path > latest)
            latest = path;
    }

    return latest;
}

void remove_old_complete_frames(const std::string& keep)
{
    for (const auto& entry : fs::directory_iterator("/tmp/livecam")) {
        if (!entry.is_regular_file())
            continue;

        if (entry.file_size() != FRAME_SIZE)
            continue;

        if (entry.path().string() != keep) {
            std::error_code ec;
            fs::remove(entry.path(), ec);
        }
    }
}

int main(int argc, char** argv)
{
    if (argc != 3) {
        std::printf("Usage: %s model.param model.bin\n", argv[0]);
        return 1;
    }

    ncnn::Net net;
    net.opt.num_threads = 4;

    if (net.load_param(argv[1]) != 0) {
        std::printf("Failed to load param\n");
        return 1;
    }

    if (net.load_model(argv[2]) != 0) {
        std::printf("Failed to load model\n");
        return 1;
    }

    std::printf("Model loaded\n");

    int server = socket(AF_INET, SOCK_STREAM, 0);

    int reuse = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        perror("bind");
        return 1;
    }

    listen(server, 1);

    while (true) {
        std::printf("Waiting for viewer on port %d...\n", PORT);

        int client = -1;

        while (client < 0) {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(server, &readfds);

            timeval timeout{};
            timeout.tv_sec = 0;
            timeout.tv_usec = 100000;

            int ready = select(server + 1, &readfds, nullptr, nullptr, &timeout);

            if (ready > 0 && FD_ISSET(server, &readfds)) {
                client = accept(server, nullptr, nullptr);
                continue;
            }

            if (ready < 0) {
                perror("select");
                break;
            }

            std::string latest = find_latest_frame();

            if (!latest.empty())
                remove_old_complete_frames(latest);
        }

        if (client < 0)
            continue;

        std::printf("Viewer connected\n");

        while (true) {
            std::string frame_path = find_latest_frame();

            if (frame_path.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                continue;
            }

            std::ifstream file(frame_path, std::ios::binary);

            std::vector<unsigned char> pixels(FRAME_SIZE);
            file.read(reinterpret_cast<char*>(pixels.data()), FRAME_SIZE);

            if (file.gcount() != static_cast<std::streamsize>(FRAME_SIZE)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                continue;
            }

            remove_old_complete_frames(frame_path);

            ncnn::Mat resized = ncnn::Mat::from_pixels_resize(
                pixels.data(),
                ncnn::Mat::PIXEL_BGRA2RGB,
                SRC_W,
                SRC_H,
                640,
                480
            );

            ncnn::Mat input;

            ncnn::copy_make_border(
                resized,
                input,
                80,
                80,
                0,
                0,
                ncnn::BORDER_CONSTANT,
                114.f
            );

            const float norm[3] = {
                1.f / 255.f,
                1.f / 255.f,
                1.f / 255.f
            };

            input.substract_mean_normalize(nullptr, norm);

            auto start = std::chrono::steady_clock::now();

            ncnn::Extractor ex = net.create_extractor();
            ex.input("in0", input);

            ncnn::Mat output;
            ex.extract("out0", output);

            auto end = std::chrono::steady_clock::now();

            double inference_ms =
                std::chrono::duration<double, std::milli>(
                    end - start
                ).count();

            int best_index = -1;
            float best_score = 0.f;

            if (output.dims == 2 && output.h >= 5) {
                const float* scores = output.row(4);

                for (int i = 0; i < output.w; ++i) {
                    if (scores[i] > best_score) {
                        best_score = scores[i];
                        best_index = i;
                    }
                }
            }

            int detected = 0;
            float x1 = 0;
            float y1 = 0;
            float x2 = 0;
            float y2 = 0;

            if (best_index >= 0 && best_score >= 0.25f) {
                const float* xs = output.row(0);
                const float* ys = output.row(1);
                const float* ws = output.row(2);
                const float* hs = output.row(3);

                float cx = xs[best_index];
                float cy = ys[best_index];
                float w = ws[best_index];
                float h = hs[best_index];

                const float scale = 0.8f;

                x1 = (cx - w / 2.f) / scale;
                y1 = (cy - h / 2.f - 80.f) / scale;
                x2 = (cx + w / 2.f) / scale;
                y2 = (cy + h / 2.f - 80.f) / scale;

                x1 = std::clamp(x1, 0.f, 799.f);
                y1 = std::clamp(y1, 0.f, 599.f);
                x2 = std::clamp(x2, 0.f, 799.f);
                y2 = std::clamp(y2, 0.f, 599.f);

                detected = 1;
            }

            char header[256];

            int header_len = std::snprintf(
                header,
                sizeof(header),
                "FRAME %zu %d %.6f %.2f %.2f %.2f %.2f %.2f\n",
                FRAME_SIZE,
                detected,
                best_score,
                x1,
                y1,
                x2,
                y2,
                inference_ms
            );

            if (!send_all(client, header, header_len))
                break;

            if (!send_all(client, pixels.data(), pixels.size()))
                break;

            std::error_code ec;
            fs::remove(frame_path, ec);
        }

        close(client);
        std::printf("Viewer disconnected\n");
    }

    close(server);
    return 0;
}
