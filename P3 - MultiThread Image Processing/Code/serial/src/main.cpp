#include <chrono>
#include <cstdlib>
#include <functional>
#include <iostream>

#include "bmp.hpp"
#include "filter.hpp"

constexpr char OUTPUT[] = "../../../Assets/Pictures/output_serial.bmp";

namespace chrono = std::chrono;
using TimeRes = chrono::duration<float, std::milli>;


TimeRes::rep applyFilter(BMP24::BMP& image, std::function<void(BMP24::BMP&)> filterFunc) {
    auto tstart = chrono::high_resolution_clock::now();
    filterFunc(image);
    auto tend = chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeRes>(tend - tstart).count();
}

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <filename>\n";
        return EXIT_FAILURE;
    }

    BMP24::BMP image;
    auto timeStart = chrono::high_resolution_clock::now();

    BMP24::BMP::ReadResult res = image.read(argv[1]);
    if (res != BMP24::BMP::ReadResult::success) {
        std::cerr << "Error opening file: #" << static_cast<int>(res) << '\n';
        return EXIT_FAILURE;
    }
    auto timeReadEnd = chrono::high_resolution_clock::now();

    auto timeFlip = applyFilter(image, [](BMP24::BMP& img) { filter::flip(img, filter::FlipType::vertical); });
    auto timePurpleHaze = applyFilter(image, [](BMP24::BMP& img) { filter::purpleHaze(img); });
    auto timeDiagonal = applyFilter(image, [](BMP24::BMP& img) { filter::diagonalHatch(img, BMP24::RGB(255, 255, 255)); });
    auto timeGaussian = applyFilter(image, [](BMP24::BMP& img) { filter::guassianBlur(img); });

    auto timeEnd = chrono::high_resolution_clock::now();

    std::cout << "Read Time: " << chrono::duration_cast<TimeRes>(timeReadEnd - timeStart).count()
              << " ms\n";
    std::cout << "Flip Time: " << timeFlip << " ms\n";
    std::cout << "Purple Haze Time: " << timePurpleHaze << " ms\n";
    std::cout << "Diagonal Hatch Time: " << timeDiagonal << " ms\n";
    std::cout << "Gaussian Blur Time: " << timeGaussian << " ms\n";
    std::cout << "Execution Time: " << chrono::duration_cast<TimeRes>(timeEnd - timeStart).count()
              << " ms\n";

    if (!image.write(OUTPUT)) {
        std::cerr << "Error writing file\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
