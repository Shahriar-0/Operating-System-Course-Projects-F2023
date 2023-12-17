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
            for (int col = 0; col < img.width() / 2; ++col) 
                std::swap(img(row, col), img(row, img.width() - 1 - col));
        }
    } else if (type == FlipType::vertical) {
        for (int row = 0; row < img.height() / 2; ++row) {
            for (int col = 0; col < img.width(); ++col) 
                std::swap(img(row, col), img(img.height() - 1 - row, col));
        }
    } else if (type == FlipType::horizontal) {
        for (int row = 0; row < img.height() / 2; ++row) {
            for (int col = 0; col < img.width(); ++col) 
                std::swap(img(row, col), img(img.height() - 1 - row, img.width() - 1 - col));
        }
    }
}

void rotate(BMP24::BMP& img, RotateType type) {
    BMP24::BMP tmp(img);
    auto clockWise = [&tmp](int row, int col) { return tmp(tmp.height() - 1 - col, row); };
    auto counterClockWise = [&tmp](int row, int col) { return tmp(col, tmp.width() - 1 - row); };
    if (!img.create(img.height(), img.width())) return;
    for (int row = 0; row < img.height(); ++row) {
        for (int col = 0; col < img.width(); ++col) {
            img(row, col) = type == RotateType::clockwise ? clockWise(row, col) : counterClockWise(row, col);
        }
    }
}

void applyColorTransformation(BMP24::BMP_View img, std::function<uint8_t(uint8_t)> transform) {
    for (int row = 0; row < img.height(); ++row) {
        for (int col = 0; col < img.width(); ++col) {
            auto& pixel = img(row, col);
            pixel = BMP24::RGB(transform(pixel.red), transform(pixel.grn), transform(pixel.blu));
        }
    }
}

void invert(BMP24::BMP_View img) {
    applyColorTransformation(img, [](uint8_t value) { return 255 - value; });
}

void grayscale(BMP24::BMP_View img) {
    applyColorTransformation(img, [](uint8_t value) {
        auto gray = (value + value + value) / 3;
        return gray;
    });
}

void blackWhite(BMP24::BMP_View img, uint8_t threshold) {
    applyColorTransformation(img, [threshold](uint8_t value) {
        const uint8_t bw = value > threshold ? 255 : 0;
        return bw;
    });
}

void applyColorTransformation(BMP24::BMP_View img, float coeff1, float coeff2, float coeff3,
                              float coeff4, float coeff5, float coeff6, float coeff7, float coeff8,
                              float coeff9) {
    for (int row = 0; row < img.height(); ++row) {
        for (int col = 0; col < img.width(); ++col) {
            auto& pixel = img(row, col);
            auto tmp = pixel;
            pixel.red =
                std::min<int>(255, (coeff1 * tmp.red) + (coeff2 * tmp.grn) + (coeff3 * tmp.blu));
            pixel.grn =
                std::min<int>(255, (coeff4 * tmp.red) + (coeff5 * tmp.grn) + (coeff6 * tmp.blu));
            pixel.blu =
                std::min<int>(255, (coeff7 * tmp.red) + (coeff8 * tmp.grn) + (coeff9 * tmp.blu));
        }
    }
}

void sepia(BMP24::BMP_View img) {
    applyColorTransformation(img, 0.393, 0.769, 0.189, 0.349, 0.686, 0.168, 0.272, 0.534, 0.131);
}

void purpleHaze(BMP24::BMP_View img) {
    applyColorTransformation(img, 0.5, 0.5, 0.3, 0.16, 0.16, 0.5, 0.6, 0.8, 0.2);
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
            pixel.red = std::clamp<int>(red, 0, 255);
            pixel.grn = std::clamp<int>(green, 0, 255);
            pixel.blu = std::clamp<int>(blue, 0, 255);
        }
    }
}

void emboss(BMP24::BMP_View img) { kernel(img, kernels[KernelType::emboss]); }

void guassianBlur(BMP24::BMP_View img) { kernel(img, knormalize(kernels[KernelType::guassianBlur])); }

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

void diagonalHatch(BMP24::BMP& img, BMP24::RGB color) {
    int halfWidth = img.width() / 2;
    int halfHeight = img.height() / 2;

    drawLine(BMP24::BMP_View(img, 0, halfWidth, halfWidth, halfHeight), 
             {0, halfHeight - 1}, {halfWidth - 1, 0}, color);
    drawLine(BMP24::BMP_View(img, 0, 0, halfWidth, halfHeight), 
             {0, halfHeight - 1}, {halfWidth - 1, 0}, color);
    drawLine(BMP24::BMP_View(img, halfHeight, 0, halfWidth, halfHeight), 
             {0, halfHeight - 1}, {halfWidth - 1, 0}, color);
    drawLine(BMP24::BMP_View(img, halfHeight, halfWidth, halfWidth, halfHeight),
             {0, halfHeight - 1}, {halfWidth - 1, 0}, color);
}

}  // namespace filter
