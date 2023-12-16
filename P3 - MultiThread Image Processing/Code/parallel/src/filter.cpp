#include "filter.hpp"

#include <cmath>

namespace filter {

void flip(bmp::BmpView img, FlipType type) {
    switch (type) {
    case FlipType::horizontal:
        for (int row = 0; row < img.height(); ++row) {
            for (int col = 0; col < img.width() / 2; ++col) {
                std::swap(img(row, col), img(row, img.width() - 1 - col));
            }
        }
        break;
    case FlipType::vertical:
        for (int row = 0; row < img.height() / 2; ++row) {
            for (int col = 0; col < img.width(); ++col) {
                std::swap(img(row, col), img(img.height() - 1 - row, col));
            }
        }
        break;
    case FlipType::oneeighty:
        for (int row = 0; row < img.height() / 2; ++row) {
            for (int col = 0; col < img.width(); ++col) {
                std::swap(img(row, col), img(img.height() - 1 - row, img.width() - 1 - col));
            }
        }
        break;
    }
}

void rotate(bmp::Bmp& img, RotateType type) {
    bmp::Bmp tmp(img);
    if (!img.create(img.height(), img.width())) return;
    switch (type) {
    case RotateType::clockwise:
        for (int row = 0; row < img.height(); ++row) {
            for (int col = 0; col < img.width(); ++col) {
                img(row, col) = tmp(tmp.height() - 1 - col, row);
            }
        }
        break;
    case RotateType::counterclockwise:
        for (int row = 0; row < img.height(); ++row) {
            for (int col = 0; col < img.width(); ++col) {
                img(row, col) = tmp(col, tmp.width() - 1 - row);
            }
        }
        break;
    }
}

void invert(bmp::BmpView img) {
    for (int row = 0; row < img.height(); ++row) {
        for (int col = 0; col < img.width(); ++col) {
            auto& pixel = img(row, col);
            pixel = bmp::RGB(255 - pixel.red, 255 - pixel.grn, 255 - pixel.blu);
        }
    }
}

void grayscale(bmp::BmpView img) {
    for (int row = 0; row < img.height(); ++row) {
        for (int col = 0; col < img.width(); ++col) {
            auto& pixel = img(row, col);
            auto gray = (pixel.red + pixel.grn + pixel.blu) / 3;
            pixel = bmp::RGB(gray, gray, gray);
        }
    }
}

void blackwhite(bmp::BmpView img, uint8_t threshold) {
    for (int row = 0; row < img.height(); ++row) {
        for (int col = 0; col < img.width(); ++col) {
            auto& pixel = img(row, col);
            auto gray = (pixel.red + pixel.grn + pixel.blu) / 3;
            const uint8_t bw = gray > threshold ? 255 : 0;
            pixel = bmp::RGB(bw, bw, bw);
        }
    }
}

void sepia(bmp::BmpView img) {
    for (int row = 0; row < img.height(); ++row) {
        for (int col = 0; col < img.width(); ++col) {
            auto& pixel = img(row, col);
            auto tmp = pixel;
            pixel.red = std::min<int>(255, (0.393 * tmp.red) + (0.769 * tmp.grn) + (0.189 * tmp.blu));
            pixel.grn = std::min<int>(255, (0.349 * tmp.red) + (0.686 * tmp.grn) + (0.168 * tmp.blu));
            pixel.blu = std::min<int>(255, (0.272 * tmp.red) + (0.534 * tmp.grn) + (0.131 * tmp.blu));
        }
    }
}

void purpleHaze(bmp::BmpView img) {
    for (int row = 0; row < img.height(); ++row) {
            for (int col = 0; col < img.width(); ++col) {
                auto& pixel = img(row, col);
                auto tmp = pixel;
                pixel.red = std::min<int>(255, (0.5 * tmp.red) + (0.5 * tmp.blu) + (0.3 * tmp.grn));
                pixel.grn = std::min<int>(255, (0.16 * tmp.red) + (0.16 * tmp.blu) + (0.5 * tmp.grn));
                pixel.blu = std::min<int>(255, (0.6 * tmp.red) + (0.8 * tmp.blu) + (0.2 * tmp.grn));
            }
        }
}

Kernel knormalize(Kernel kernel) {
    constexpr float epsilon = 1e-3;
    float sum = 0;
    for (int i = 0; i < 9; ++i) {
        sum += kernel[i];
    }
    if (sum >= -epsilon && sum <= epsilon) return kernel;
    for (int i = 0; i < 9; ++i) {
        kernel[i] /= sum;
    }
    return kernel;
}

void kernel(bmp::BmpView img, Kernel kernel) {
    static const int dirX[] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
    static const int dirY[] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};

    const bmp::Bmp imgCpy = img;

    auto inImage = [&imgCpy](int row, int col) -> bool {
        return row >= 0 && row < imgCpy.height() && col >= 0 && col < imgCpy.width();
    };

    for (int row = 0; row < img.height(); ++row) {
        for (int col = 0; col < img.width(); ++col) {
            float red = 0, green = 0, blue = 0;
            for (int i = 0; i < 9; ++i) {
                bmp::RGB pixel = imgCpy(row, col);
                if (inImage(row + dirX[i], col + dirY[i])) {
                    pixel = imgCpy(row + dirX[i], col + dirY[i]);
                }
                red += kernel[i] * pixel.red;
                green += kernel[i] * pixel.grn;
                blue += kernel[i] * pixel.blu;
            }

            auto& pixel = img(row, col);
            pixel.red = std::min(255, std::max<int>(0, red));
            pixel.grn = std::min(255, std::max<int>(0, green));
            pixel.blu = std::min(255, std::max<int>(0, blue));
        }
    }
}

void emboss(bmp::BmpView img) {
    static const Kernel emboss = {-2, -1, 0, -1, 1, 1, 0, 1, 2};
    kernel(img, emboss);
}

void guassianblur(bmp::BmpView img) {
    static const Kernel guassian = knormalize({1, 2, 1, 2, 4, 2, 1, 2, 1});
    kernel(img, guassian);
}

void boxblur(bmp::BmpView img) {
    static const Kernel boxblur = knormalize({1, 1, 1, 1, 1, 1, 1, 1, 1});
    kernel(img, boxblur);
}

void sharpen(bmp::BmpView img) {
    static const Kernel sharpen = {0, -1, 0, -1, 5, -1, 0, -1, 0};
    kernel(img, sharpen);
}

void edgedetect(bmp::BmpView img) {
    static const Kernel edgedetect = {-1, -1, -1, -1, 8, -1, -1, -1, -1};
    kernel(img, edgedetect);
}

void drawline(bmp::BmpView img, Point p1, Point p2, bmp::RGB color) {
    int dx = p2.x - p1.x;
    int dy = p2.y - p1.y;
    int steps = std::max(std::abs(dx), std::abs(dy));

    float xinc = dx / static_cast<float>(steps);
    float yinc = dy / static_cast<float>(steps);

    float x = p1.x;
    float y = p1.y;

    for (int i = 0; i <= steps; ++i) {
        img(y, x) = color;
        x += xinc;
        y += yinc;
    }
}

void diamond(bmp::BmpView img, bmp::RGB color) {
    const Point top = {img.width() / 2, 0};
    const Point left = {0, img.height() / 2};
    const Point right = {img.width() - 1, img.height() / 2};
    const Point bottom = {img.width() / 2, img.height() - 1};
    drawline(img, left, top, color);
    drawline(img, bottom, right, color);
    drawline(img, top, right, color);
    drawline(img, left, bottom, color);

    // method 2:
    // const int h = img.height();
    // const float m = static_cast<float>(img.height()) / img.width();
    // for (int row = 0; row < img.height(); ++row) {
    //     for (int col = 0; col < img.width(); ++col) {
    //         if (row == static_cast<int>(-m * col + h / 2) ||
    //             row == static_cast<int>(-m * col + 3 * h / 2) ||
    //             row == static_cast<int>(+m * col - h / 2) ||
    //             row == static_cast<int>(+m * col + h / 2)) {
    //             img(row, col) = color;
    //         }
    //     }
    // }
}

void diagonalHatch(bmp::BmpView img, bmp::RGB color) {
    int midWidth = img.width() / 2;
    int midHeight = img.height() / 2;
    int width = img.width() - 1;
    int height = img.height() - 1;

    drawline(img, {0, height}, {0, width}, color);
}

} // namespace filter
