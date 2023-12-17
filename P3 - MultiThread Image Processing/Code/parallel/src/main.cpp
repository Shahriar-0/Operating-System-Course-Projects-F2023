#include <pthread.h>

#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>

#include "bmp.hpp"
#include "filter.hpp"
#include "pfilter.hpp"
#include "pool.hpp"

constexpr char OUTPUT[] = "../../../Assets/Pictures/output_parallel.bmp";

namespace chrono = std::chrono;
using TimeRes = chrono::duration<float, std::milli>;

TimeRes::rep flip(BMP24::BMP& image, thread::Pool& pool) {
    auto timeStart = chrono::high_resolution_clock::now();
    const int portion = image.width() / pool.count();

    std::array<BMP24::BMP_View, THREAD_COUNT> views;
    for (unsigned i = 0; i < pool.count(); ++i) {
        views[i] = BMP24::BMP_View(image, 0, i * portion, portion, image.height());
    }

    auto flipHorizontal = [](BMP24::BMP_View view) { filter::flip(view, filter::FlipType::vertical); };

    for (unsigned i = 0; i < pool.count(); ++i) {
        pool.add(new pfilter::FilterTask(views[i], flipHorizontal));
    }
    pool.waitForTasks();

    auto timeEnd = chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeRes>(timeEnd - timeStart).count();
}

TimeRes::rep purpleHaze(BMP24::BMP& image, thread::Pool& pool) {
    auto timeStart = chrono::high_resolution_clock::now();
    const int portion = image.height() / pool.count();

    std::array<BMP24::BMP_View, THREAD_COUNT> views;
    for (unsigned i = 0; i < pool.count(); ++i) {
        views[i] = BMP24::BMP_View(image, i * portion, 0, image.width(), portion);
    }

    for (unsigned i = 0; i < pool.count(); ++i) {
        pool.add(new pfilter::FilterTask(views[i], filter::purpleHaze));
    }
    pool.waitForTasks();

    auto timeEnd = chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeRes>(timeEnd - timeStart).count();
}

TimeRes::rep diagonalHatch(BMP24::BMP& image, thread::Pool& pool) {
    auto timeStart = chrono::high_resolution_clock::now();

    std::array<BMP24::BMP_View, 4> views;
    for (int i = 0; i < 4; ++i) {
        auto row = (i / 2) ? image.height() / 2 : 0;
        auto col = (i % 2) ? image.width() / 2 : 0;
        views[i] = BMP24::BMP_View(image, row, col, image.width() / 2, image.height() / 2);
    }

    auto Diagonal = [](BMP24::BMP_View view) {
        filter::drawLine(view, filter::Point{0, view.height() - 1},
                         filter::Point{view.width() - 1, 0}, BMP24::RGB(255, 255, 255));
    };

    for (int i = 0; i < 4; ++i) {
        pool.add(new pfilter::FilterTask(views[i], Diagonal));
    }
    pool.waitForTasks();

    auto timeEnd = chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeRes>(timeEnd - timeStart).count();
}

TimeRes::rep gaussianBlur(BMP24::BMP& image, thread::Pool& pool) {
    auto timeStart = chrono::high_resolution_clock::now();
    const int portion = image.height() / pool.count();

    std::array<BMP24::BMP_View, THREAD_COUNT> viewsOrig;
    std::array<BMP24::BMP, THREAD_COUNT> cuts;
    std::array<BMP24::BMP_View, THREAD_COUNT> viewsCuts;
    for (unsigned i = 0; i < pool.count(); ++i) {
        viewsOrig[i] = BMP24::BMP_View(image, i * portion, 0, image.width(), portion);
        cuts[i] = BMP24::BMP(viewsOrig[i]);
        viewsCuts[i] = cuts[i];
    }

    for (unsigned i = 0; i < pool.count(); ++i) {
        pool.add(new pfilter::FilterTask(viewsCuts[i], filter::guassianBlur));
    }
    pool.waitForTasks();
    for (unsigned i = 0; i < pool.count(); ++i) {
        pool.add(new pfilter::ReplaceTask(viewsOrig[i], viewsCuts[i]));
    }
    pool.waitForTasks();

    auto timeEnd = chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeRes>(timeEnd - timeStart).count();
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

    auto timeFlip = flip(image, pool);
    auto timePurpleHaze = purpleHaze(image, pool);
    auto timeDiagonal = diagonalHatch(image, pool);
    auto timeGaussian = gaussianBlur(image, pool);

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
