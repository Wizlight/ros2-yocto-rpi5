#include <net.h>
#include <cstdio>

int main(int argc, char** argv)
{
    if (argc != 3) {
        std::printf("Usage: %s model.ncnn.param model.ncnn.bin\n", argv[0]);
        return 1;
    }

    ncnn::Net net;
    net.opt.num_threads = 4;

    if (net.load_param(argv[1]) != 0) {
        std::printf("Failed to load param file\n");
        return 1;
    }

    if (net.load_model(argv[2]) != 0) {
        std::printf("Failed to load model file\n");
        return 1;
    }

    ncnn::Mat input(640, 640, 3);
    input.fill(0.f);

    ncnn::Extractor ex = net.create_extractor();

    if (ex.input("in0", input) != 0) {
        std::printf("Failed to set input\n");
        return 1;
    }

    ncnn::Mat output;

    if (ex.extract("out0", output) != 0) {
        std::printf("Failed to extract output\n");
        return 1;
    }

    std::printf("Inference OK\n");
    std::printf(
        "out0: dims=%d w=%d h=%d d=%d c=%d total=%zu\n",
        output.dims,
        output.w,
        output.h,
        output.d,
        output.c,
        output.total()
    );

    return 0;
}
