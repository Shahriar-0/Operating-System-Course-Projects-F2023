#include "filter.hpp"

#include <cmath>

namespace filter {

std::unordered_map<KernelType, Kernel> kernels = {
    {KernelType::emboss, Kernel{-2, -1, 0, -1, 1, 1, 0, 1, 2}},
    {KernelType::guassianBlur, Kernel{1, 2, 1, 2, 4, 2, 1, 2, 1}},
    {KernelType::boxBlur, Kernel{1, 1, 1, 1, 1, 1, 1, 1, 1}},
    {KernelType::sharpen, Kernel{0, -1, 0, -1, 5, -1, 0, -1, 0}},
    {KernelType::edgeDetect, Kernel{0, 1, 0, 1, -4, 1, 0, 1, 0}},
};

void flip(BMP24::BMP_View img, FlipType type) {
    if (type == FlipType::oneeighty) {
        for (int row = 0; row < img.height(); ++row) {
            for (int col = 0; col < img.width() / 2; ++col) {
                std::swap(img(row, col), img(row, img.width() - 1 - col));
            }
        }
    } else if (type == FlipType::vertical) {
        for (int row = 0; row < img.height() / 2; ++row) {
            for (int col = 0; col < img.width(); ++col) {
                std::swap(img(row, col), img(img.height() - 1 - row, col));
            }
        }
    } else if (type == FlipType::horizontal) {
        for (int row = 0; row < img.height() / 2; ++row) {
            for (int col = 0; col < img.width(); ++col) {
                std::swap(img(row, col), img(img.height() - 1 - row, img.width() - 1 - col));
            }
        }
    }
}

void rotate(BMP24::BMP& img, RotateType type) {
    BMP24::BMP tmp(img);
    if (!img.create(img.height(), img.width())) return;
    if (type == RotateType::clockwise) {
        for (int row = 0; row < img.height(); ++row) {
            for (int col = 0; col < img.width(); ++col) {
                img(row, col) = tmp(tmp.height() - 1 - col, row);
            }
        }
    } else if (type == RotateType::counterclockwise) {
        for (int row = 0; row < img.height(); ++row) {
            for (int col = 0; col < img.width(); ++col) {
                img(row, col) = tmp(col, tmp.width() - 1 - row);
            }
        }
    }
}

void invert(BMP24::BMP_View img) {
    for (int row = 0; row < img.height(); ++row) {
        for (int col = 0; col < img.width(); ++col) {
            auto& pixel = img(row, col);
            pixel = BMP24::RGB(255 - pixel.red, 255 - pixel.grn, 255 - pixel.blu);
        }
    }
}

void grayscale(BMP24::BMP_View img) {
    for (int row = 0; row < img.height(); ++row) {
        for (int col = 0; col < img.width(); ++col) {
            auto& pixel = img(row, col);
            auto gray = (pixel.red + pixel.grn + pixel.blu) / 3;
            pixel = BMP24::RGB(gray, gray, gray);
        }
    }
}

void blackWhite(BMP24::BMP_View img, uint8_t threshold) {
    for (int row = 0; row < img.height(); ++row) {
        for (int col = 0; col < img.width(); ++col) {
            auto& pixel = img(row, col);
            auto gray = (pixel.red + pixel.grn + pixel.blu) / 3;
            const uint8_t bw = gray > threshold ? 255 : 0;
            pixel = BMP24::RGB(bw, bw, bw);
        }
    }
}

void sepia(BMP24::BMP_View img) {
    for (int row = 0; row < img.height(); ++row) {
        for (int col = 0; col < img.width(); ++col) {
            auto& pixel = img(row, col);
            auto tmp = pixel;
            pixel.red =
                std::min<int>(255, (0.393 * tmp.red) + (0.769 * tmp.grn) + (0.189 * tmp.blu));
            pixel.grn =
                std::min<int>(255, (0.349 * tmp.red) + (0.686 * tmp.grn) + (0.168 * tmp.blu));
            pixel.blu =
                std::min<int>(255, (0.272 * tmp.red) + (0.534 * tmp.grn) + (0.131 * tmp.blu));
        }
    }
}

void purpleHaze(BMP24::BMP_View img) {
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
    for (int i = 0; i < 9; ++i) sum += kernel[i];
    if (sum >= -epsilon && sum <= epsilon) return kernel;
    for (int i = 0; i < 9; ++i) kernel[i] /= sum;
    return kernel;
}

void kernel(BMP24::BMP_View img, Kernel kernel) {
    static const int dirX[] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
    static const int dirY[] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};

    const BMP24::BMP imgCpy = img;

    auto inImage = [&imgCpy](int row, int col) -> bool {
        return row >= 0 && row < imgCpy.height() && col >= 0 && col < imgCpy.width();
    };

    for (int row = 0; row < img.height(); ++row) {
        for (int col = 0; col < img.width(); ++col) {
            float red = 0, green = 0, blue = 0;
            for (int i = 0; i < 9; ++i) {
                BMP24::RGB pixel = imgCpy(row, col);
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

void emboss(BMP24::BMP_View img) { kernel(img, kernels[KernelType::emboss]); }

void guassianBlur(BMP24::BMP_View img) {
    kernel(img, knormalize(kernels[KernelType::guassianBlur]));
}

void boxBlur(BMP24::BMP_View img) { kernel(img, knormalize(kernels[KernelType::boxBlur])); }

void sharpen(BMP24::BMP_View img) { kernel(img, kernels[KernelType::sharpen]); }

void edgeDetect(BMP24::BMP_View img) { kernel(img, kernels[KernelType::edgeDetect]); }

void drawLine(BMP24::BMP_View img, Point p1, Point p2, BMP24::RGB color) {
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

void diagonalHatch(BMP24::BMP_View img, BMP24::RGB color) {
    int width = img.width() - 1;
    int height = img.height() - 1;
    drawLine(img, {0, height}, {0, width}, color);
}

}  // namespace filter
