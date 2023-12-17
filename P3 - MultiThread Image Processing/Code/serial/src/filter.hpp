#ifndef FILTER_HPP_INCLUDE
#define FILTER_HPP_INCLUDE

#include <algorithm>
#include <array>
#include <iostream>
#include <unordered_map>
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

enum class KernelType {
    emboss,
    guassianBlur,
    boxBlur,
    sharpen,
    edgeDetect,
    purpleHaze,
};

using Kernel = std::array<float, 9>;

struct Point {
    int x;
    int y;
};

void flip(BMP24::BMP_View img, FlipType type);
void rotate(BMP24::BMP& img, RotateType type);

void invert(BMP24::BMP_View img);
void grayscale(BMP24::BMP_View img);
void blackWhite(BMP24::BMP_View img, uint8_t threshold = 127);
void sepia(BMP24::BMP_View img);

Kernel knormalize(Kernel kernel);
void kernel(BMP24::BMP_View img, Kernel kernel);

void emboss(BMP24::BMP_View img);
void guassianBlur(BMP24::BMP_View img);
void boxBlur(BMP24::BMP_View img);
void sharpen(BMP24::BMP_View img);
void edgeDetect(BMP24::BMP_View img);
void purpleHaze(BMP24::BMP_View img);

void drawLine(BMP24::BMP_View img, Point p1, Point p2, BMP24::RGB color);
void diagonalHatch(BMP24::BMP& img, BMP24::RGB color);

}  // namespace filter

#endif  // FILTER_HPP_INCLUDE
