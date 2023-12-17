#ifndef FILTER_HPP_INCLUDE
#define FILTER_HPP_INCLUDE

#include <algorithm>
#include <array>
#include <iostream>
#include <utility>
#include <unordered_map>

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

void flip(bmp::BmpView img, FlipType type);
void rotate(bmp::Bmp& img, RotateType type);

void invert(bmp::BmpView img);
void grayscale(bmp::BmpView img);
void blackWhite(bmp::BmpView img, uint8_t threshold = 127);
void sepia(bmp::BmpView img);

Kernel knormalize(Kernel kernel);
void kernel(bmp::BmpView img, Kernel kernel);

void emboss(bmp::BmpView img);
void guassianBlur(bmp::BmpView img);
void boxBlur(bmp::BmpView img);
void sharpen(bmp::BmpView img);
void edgeDetect(bmp::BmpView img);
void purpleHaze(bmp::BmpView img);

void drawLine(bmp::BmpView img, Point p1, Point p2, bmp::RGB color);
void diagonalHatch(bmp::Bmp& img, bmp::RGB color);

}  // namespace filter

#endif  // FILTER_HPP_INCLUDE
