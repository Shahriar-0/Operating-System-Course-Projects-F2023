#include <chrono>
#include <cstdlib>
#include <iostream>

#include "bmp.hpp"
#include "filter.hpp"

constexpr char OUTPUT[] = "output.bmp";

namespace chrono = std::chrono;
using TimeRes = chrono::duration<float, std::milli>;

TimeRes::rep flip(bmp::Bmp& image) {
    auto tstart = chrono::high_resolution_clock::now();
    // filter::flip(image, filter::FlipType::horizontal);
    filter::flip(image, filter::FlipType::vertical);
    auto tend = chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeRes>(tend - tstart).count();
}

TimeRes::rep purpleHaze(bmp::Bmp& image) {
    auto tstart = chrono::high_resolution_clock::now();
    filter::purpleHaze(image);
    auto tend = chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeRes>(tend - tstart).count();
}

TimeRes::rep diagonalHatch(bmp::Bmp& image) {
    auto tstart = chrono::high_resolution_clock::now();
    filter::diagonalHatch(image, bmp::RGB(255, 255, 255));
    auto tend = chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeRes>(tend - tstart).count();
}

TimeRes::rep gaussianBlur(bmp::Bmp& image) {
    auto tstart = chrono::high_resolution_clock::now();
    filter::guassianblur(image);
    auto tend = chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeRes>(tend - tstart).count();
}

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <filename>\n";
        return EXIT_FAILURE;
    }

    bmp::Bmp image;
    auto timeStart = chrono::high_resolution_clock::now();

    bmp::Bmp::ReadResult res = image.read(argv[1]);
    if (res != bmp::Bmp::ReadResult::success) {
        std::cerr << "Error opening file: #" << static_cast<int>(res) << '\n';
        return EXIT_FAILURE;
    }
    auto timeReadEnd = chrono::high_resolution_clock::now();

    auto timeFlip = flip(image);
    auto timeEmboss = purpleHaze(image);
    auto timeDiagonal = diagonalHatch(image);
    auto timeGaussian = gaussianBlur(image);

    auto timeEnd = chrono::high_resolution_clock::now();

    std::cout << "Read Time: " << chrono::duration_cast<TimeRes>(timeReadEnd - timeStart).count() << " ms\n";
    std::cout << "Flip Time: " << timeFlip << " ms\n";
    std::cout << "Purple Haze Time: " << timeEmboss << " ms\n";
    std::cout << "Diagonal Hatch Time: " << timeDiagonal << " ms\n";
    std::cout << "Gaussian Blur Time: " << timeGaussian << " ms\n";
    std::cout << "Execution Time: " << chrono::duration_cast<TimeRes>(timeEnd - timeStart).count() << '\n';

    if (!image.write(OUTPUT)) {
        std::cerr << "Error writing file\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
