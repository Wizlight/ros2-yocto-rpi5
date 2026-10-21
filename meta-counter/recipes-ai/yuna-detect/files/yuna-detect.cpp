#include <net.h>
#include <mat.h>

#include <cstdio>
#include <vector>
#include <fstream>
#include <algorithm>
#include <chrono>

int main(int argc, char** argv)
{
    if (argc != 4) {
        std::printf(
            "Usage: %s model.param model.bin frame.raw\n",
            argv[0]
        );
        return 1;
    }

    const int src_w = 800;
    const int src_h = 600;
    const int target = 640;

    const size_t expected_size = src_w * src_h * 4;

    std::ifstream file(argv[3], std::ios::binary | std::ios::ate);
    if (!file) {
        std::printf("Failed to open raw frame\n");
        return 1;
    }

    const size_t size = file.tellg();
    if (size < expected_size) {
        std::printf("Raw frame too small: %zu bytes\n", size);
        return 1;
    }

    file.seekg(0);

    std::vector<unsigned char> pixels(expected_size);
    file.read(reinterpret_cast<char*>(pixels.data()), expected_size);

    // 800x600 -> 640x480, keeping aspect ratio.
    ncnn::Mat resized = ncnn::Mat::from_pixels_resize(
        pixels.data(),
        ncnn::Mat::PIXEL_BGRA2RGB,
        src_w,
        src_h,
        640,
        480
    );

    // YOLO letterbox: 80 pixels top + 80 bottom.
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

    // 0..255 -> 0..1
    const float norm[3] = {
        1.f / 255.f,
        1.f / 255.f,
        1.f / 255.f
    };

    input.substract_mean_normalize(nullptr, norm);

    ncnn::Net net;
    net.opt.num_threads = 4;

    if (net.load_param(argv[1]) != 0) {
        std::printf("Failed to load model param\n");
        return 1;
    }

    if (net.load_model(argv[2]) != 0) {
        std::printf("Failed to load model bin\n");
        return 1;
    }

    ncnn::Extractor ex = net.create_extractor();

    auto start = std::chrono::steady_clock::now();

    if (ex.input("in0", input) != 0) {
        std::printf("Failed to set model input\n");
        return 1;
    }

    ncnn::Mat output;

    if (ex.extract("out0", output) != 0) {
        std::printf("Failed to extract model output\n");
        return 1;
    }

    auto end = std::chrono::steady_clock::now();

    double inference_ms =
        std::chrono::duration<double, std::milli>(end - start).count();

    if (output.dims != 2 || output.h < 5) {
        std::printf(
            "Unexpected output: dims=%d w=%d h=%d\n",
            output.dims,
            output.w,
            output.h
        );
        return 1;
    }

    const float* xs = output.row(0);
    const float* ys = output.row(1);
    const float* ws = output.row(2);
    const float* hs = output.row(3);
    const float* scores = output.row(4);

    int best_index = -1;
    float best_score = 0.f;

    for (int i = 0; i < output.w; ++i) {
        if (scores[i] > best_score) {
            best_score = scores[i];
            best_index = i;
        }
    }

    std::printf("Inference: %.2f ms (%.2f FPS equivalent)\n",
                inference_ms,
                1000.0 / inference_ms);

    if (best_index < 0 || best_score < 0.25f) {
        std::printf("Yuna not detected. Best confidence: %.3f\n", best_score);
        return 0;
    }

    // Coordinates are relative to the 640x640 letterboxed image.
    float cx = xs[best_index];
    float cy = ys[best_index];
    float w = ws[best_index];
    float h = hs[best_index];

    // Undo letterbox:
    // original 800x600 -> scaled by 0.8 -> 640x480 + 80px top/bottom.
    const float scale = 0.8f;

    float x1 = (cx - w / 2.f) / scale;
    float y1 = (cy - h / 2.f - 80.f) / scale;
    float x2 = (cx + w / 2.f) / scale;
    float y2 = (cy + h / 2.f - 80.f) / scale;

    x1 = std::clamp(x1, 0.f, 799.f);
    y1 = std::clamp(y1, 0.f, 599.f);
    x2 = std::clamp(x2, 0.f, 799.f);
    y2 = std::clamp(y2, 0.f, 599.f);

    std::printf("Yuna detected!\n");
    std::printf("Confidence: %.3f\n", best_score);
    std::printf(
        "Box: x1=%.1f y1=%.1f x2=%.1f y2=%.1f\n",
        x1, y1, x2, y2
    );

    return 0;
}
