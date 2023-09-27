#include <pthread.h>

#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>

#include "bmp24.hpp"
#include "filter.hpp"
#include "pfilter.hpp"
#include "pool.hpp"

constexpr char OUTPUT[] = "output.bmp";

namespace chrono = std::chrono;
using TimeRes = chrono::duration<float, std::milli>;

TimeRes::rep flipHorizontal(bmp::Bmp &image, thread::Pool &pool)
{
    auto timeStart = chrono::high_resolution_clock::now();
    const int portion = image.height() / pool.count();

    std::array<bmp::BmpView, THREAD_COUNT> views;
    for (unsigned i = 0; i < pool.count(); ++i)
    {
        views[i] = bmp::BmpView(image, i * portion, 0, image.width(), portion);
    }

    auto flip = [](bmp::BmpView view)
    {
        filter::flip(view, filter::FlipType::horizontal);
    };

    for (unsigned i = 0; i < pool.count(); ++i)
    {
        pool.add(new pfilter::FilterTask(views[i], flip));
    }
    pool.waitForTasks();

    auto timeEnd = chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeRes>(timeEnd - timeStart).count();
}

TimeRes::rep flipVertical(bmp::Bmp &image, thread::Pool &pool)
{
    auto timeStart = chrono::high_resolution_clock::now();
    const int portion = image.width() / pool.count();

    std::array<bmp::BmpView, THREAD_COUNT> views;
    for (unsigned i = 0; i < pool.count(); ++i)
    {
        views[i] = bmp::BmpView(image, 0, i * portion, portion, image.height());
    }

    auto flip = [](bmp::BmpView view)
    {
        filter::flip(view, filter::FlipType::vertical);
    };

    for (unsigned i = 0; i < pool.count(); ++i)
    {
        pool.add(new pfilter::FilterTask(views[i], flip));
    }
    pool.waitForTasks();

    auto timeEnd = chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeRes>(timeEnd - timeStart).count();
}

TimeRes::rep sharpen(bmp::Bmp &image, thread::Pool &pool)
{
    auto timeStart = chrono::high_resolution_clock::now();
    const int portion = image.height() / pool.count();

    std::array<bmp::BmpView, THREAD_COUNT> viewsOrig;
    std::array<bmp::Bmp, THREAD_COUNT> cuts;
    std::array<bmp::BmpView, THREAD_COUNT> viewsCuts;
    for (unsigned i = 0; i < pool.count(); ++i)
    {
        viewsOrig[i] = bmp::BmpView(image, i * portion, 0, image.width(), portion);
        cuts[i] = bmp::Bmp(viewsOrig[i]);
        viewsCuts[i] = cuts[i];
    }

    for (unsigned i = 0; i < pool.count(); ++i)
    {
        pool.add(new pfilter::FilterTask(viewsCuts[i], filter::sharpen));
    }
    pool.waitForTasks();
    for (unsigned i = 0; i < pool.count(); ++i)
    {
        pool.add(new pfilter::ReplaceTask(viewsOrig[i], viewsCuts[i]));
    }
    pool.waitForTasks();

    auto timeEnd = chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeRes>(timeEnd - timeStart).count();
}

TimeRes::rep sepia(bmp::Bmp &image, thread::Pool &pool)
{
    auto timeStart = chrono::high_resolution_clock::now();
    const int portion = image.height() / pool.count();

    std::array<bmp::BmpView, THREAD_COUNT> views;
    for (unsigned i = 0; i < pool.count(); ++i)
    {
        views[i] = bmp::BmpView(image, i * portion, 0, image.width(), portion);
    }

    auto flipHorizontal = [](bmp::BmpView view)
    {
        filter::sepia(view);
    };

    for (unsigned i = 0; i < pool.count(); ++i)
    {
        pool.add(new pfilter::FilterTask(views[i], flipHorizontal));
    }
    pool.waitForTasks();

    auto timeEnd = chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeRes>(timeEnd - timeStart).count();
}

TimeRes::rep drawX(bmp::Bmp &image, thread::Pool &pool)
{
    auto timeStart = chrono::high_resolution_clock::now();

    std::array<bmp::BmpView, 4> views;
    for (int i = 0; i < 2; ++i)
    {
        views[i] = bmp::BmpView(image, 0, 0, image.width(), image.height());
    }

    auto XDiag = [](bmp::BmpView view)
    {
        filter::drawline(view,
                         filter::Point{0, 0},
                         filter::Point{view.width() - 1, view.height()},
                         bmp::RGB(255, 255, 255));
    };
    auto XAntiDiag = [](bmp::BmpView view)
    {
        filter::drawline(view,
                         filter::Point{view.width(), 0},
                         filter::Point{1, view.height()},
                         bmp::RGB(255, 255, 255));
    };

    pool.add(new pfilter::FilterTask(views[0], XAntiDiag));
    pool.add(new pfilter::FilterTask(views[1], XDiag));
    pool.waitForTasks();

    auto timeEnd = chrono::high_resolution_clock::now();
    return chrono::duration_cast<TimeRes>(timeEnd - timeStart).count();
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

    auto timeFlipHorizontal = flipHorizontal(image, pool);
    auto timeFlipVertical = flipVertical(image, pool);
    auto timeSharpen = sharpen(image, pool);
    auto timeSepia = sepia(image, pool);
    auto timeDrawX = drawX(image, pool);

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