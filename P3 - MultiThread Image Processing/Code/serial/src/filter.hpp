#ifndef FILTER_HPP_INCLUDE
#define FILTER_HPP_INCLUDE

#include <array>
#include <iostream>
#include <algorithm>
#include <utility>
#include "bmp.hpp"

namespace filter {

enum class FlipType {
    vertical,
    horizontal,
    oneeighty,
};

enum class RotateType {
    clockwise,
    counterclockwise,
};

using Kernel = std::array<float, 9>;

struct Point {
    int x;
    int y;
};

void flip(bmp::BmpView img, FlipType type);
void rotate(bmp::Bmp& img, RotateType type);

void invert(bmp::BmpView img);
void grayscale(bmp::BmpView img);
void blackwhite(bmp::BmpView img, uint8_t threshold = 127);
void sepia(bmp::BmpView img);

Kernel knormalize(Kernel kernel);
void kernel(bmp::BmpView img, Kernel kernel);

void emboss(bmp::BmpView img);
void guassianblur(bmp::BmpView img);
void boxblur(bmp::BmpView img);
void sharpen(bmp::BmpView img);
void edgedetect(bmp::BmpView img);
void purpleHaze(bmp::BmpView img);

void drawline(bmp::BmpView img, Point p1, Point p2, bmp::RGB color);
void diamond(bmp::BmpView img, bmp::RGB color);
void diagonalHatch(bmp::Bmp& img, bmp::RGB color);

} // namespace filter

#endif // FILTER_HPP_INCLUDE
