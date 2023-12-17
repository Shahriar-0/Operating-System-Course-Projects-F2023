#include "utils.hpp"

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

    auto timeFlip = flip(image, pool);
    auto timePurpleHaze = purpleHaze(image, pool);
    auto timeDiagonal = diagonalHatch(image, pool);
    auto timeGaussian = gaussianBlur(image, pool);

    auto timeEnd = chrono::high_resolution_clock::now();

    // clang-format off
    std::cout << "Read Time: "           << chrono::duration_cast<TimeRes>(timeReadEnd - timeStart).count() << " ms\n";
    std::cout << "Flip Time: "           << timeFlip                                                        << " ms\n";
    std::cout << "Purple Haze Time: "    << timePurpleHaze                                                  << " ms\n";
    std::cout << "Diagonal Hatch Time: " << timeDiagonal                                                    << " ms\n";
    std::cout << "Gaussian Blur Time: "  << timeGaussian                                                    << " ms\n";
    std::cout << "Execution Time: "      << chrono::duration_cast<TimeRes>(timeEnd - timeStart).count()     << " ms\n";
    // clang-format on

    if (image.write(OUTPUT) != BMP24::BMP::WriteResult::success) {
        std::cerr << "Error writing file: #" << static_cast<int>(res) << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
