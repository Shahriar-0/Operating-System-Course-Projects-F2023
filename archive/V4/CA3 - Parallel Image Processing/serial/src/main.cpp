#include <chrono>
#include <cstdlib>
#include <iostream>

#include "bmp24.hpp"
#include "filter.hpp"

constexpr char OUTPUT[] = "output.bmp";

namespace chrono = std::chrono;
using TimeRes = chrono::duration<float, std::milli>;

TimeRes::rep flipHorizontal(bmp::Bmp &image)
{
    auto tstart = chrono::high_resolution_clock::now();
    filter::flip(image, filter::FlipType::horizontal);
    auto tend = chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeRes>(tend - tstart).count();
}

TimeRes::rep flipVertical(bmp::Bmp &image)
{
    auto tstart = chrono::high_resolution_clock::now();
    filter::flip(image, filter::FlipType::vertical);
    auto tend = chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeRes>(tend - tstart).count();
}

TimeRes::rep sharpen(bmp::Bmp &image)
{
    auto tstart = chrono::high_resolution_clock::now();
    filter::sharpen(image);
    auto tend = chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeRes>(tend - tstart).count();
}

TimeRes::rep sepia(bmp::Bmp &image)
{
    auto tstart = chrono::high_resolution_clock::now();
    filter::sepia(image);
    auto tend = chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeRes>(tend - tstart).count();
}

TimeRes::rep drawX(bmp::Bmp &image)
{
    auto tstart = chrono::high_resolution_clock::now();
    filter::xshape(image, bmp::RGB(255, 255, 255));
    auto tend = chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeRes>(tend - tstart).count();
}

int main(int argc, const char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "usage: " << argv[0] << " <filename>\n";
        return EXIT_FAILURE;
    }

    bmp::Bmp image;
    auto timeStart = chrono::high_resolution_clock::now();

    bmp::Bmp::ReadResult res = image.read(argv[1]);
    if (res != bmp::Bmp::ReadResult::success)
    {
        std::cerr << "Error opening file: #" << static_cast<int>(res) << '\n';
        return EXIT_FAILURE;
    }
    auto timeReadEnd = chrono::high_resolution_clock::now();

    auto timeFlipHorizontal = flipHorizontal(image);
    auto timeFlipVertical = flipVertical(image);
    auto timeSharpen = sharpen(image);
    auto timeSepia = sepia(image);
    auto timeDrawX = drawX(image);

    auto timeEnd = chrono::high_resolution_clock::now();

    std::cout << "Read Time: " << chrono::duration_cast<TimeRes>(timeReadEnd - timeStart).count() << " ms\n";
    std::cout << "Flip Horizontal Time: " << timeFlipHorizontal << " ms\n";
    std::cout << "Flip Vertical Time: " << timeFlipVertical << " ms\n";
    std::cout << "Sharpen Time: " << timeSharpen << " ms\n";
    std::cout << "Sepia Time: " << timeSepia << " ms\n";
    std::cout << "Draw X-Shape Time: " << timeDrawX << " ms\n";
    std::cout << "Execution Time: " << chrono::duration_cast<TimeRes>(timeEnd - timeStart).count() << " ms\n";

    if (!image.write(OUTPUT))
    {
        std::cerr << "Error writing file\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}