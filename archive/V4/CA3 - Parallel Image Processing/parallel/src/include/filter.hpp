#ifndef FILTER_HPP
#define FILTER_HPP

#include <array>

#include "bmp24.hpp"

namespace filter
{

    enum class FlipType
    {
        vertical,
        horizontal,
    };

    using Kernel = std::array<float, 9>;

    struct Point
    {
        int x;
        int y;
    };

    Kernel knormalize(Kernel kernel);
    void kernel(bmp::BmpView img, Kernel kernel);

    void flip(bmp::BmpView img, FlipType type);
    void sharpen(bmp::BmpView img);
    void sepia(bmp::BmpView img);
    void drawline(bmp::BmpView img, Point p1, Point p2, bmp::RGB color);
    void xshape(bmp::BmpView img, bmp::RGB color);

} // namespace filter

#endif // FILTER_HPP_INCLUDE