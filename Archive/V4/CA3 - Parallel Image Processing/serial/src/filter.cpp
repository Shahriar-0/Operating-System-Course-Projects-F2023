#include "filter.hpp"

#include <cmath>

namespace filter
{

    void flip(bmp::BmpView img, FlipType type)
    {
        switch (type)
        {
        case FlipType::horizontal:
            for (int row = 0; row < img.height(); ++row)
            {
                for (int col = 0; col < img.width() / 2; ++col)
                {
                    std::swap(img(row, col), img(row, img.width() - 1 - col));
                }
            }
            break;
        case FlipType::vertical:
            for (int row = 0; row < img.height() / 2; ++row)
            {
                for (int col = 0; col < img.width(); ++col)
                {
                    std::swap(img(row, col), img(img.height() - 1 - row, col));
                }
            }
            break;
        }
    }

    Kernel knormalize(Kernel kernel)
    {
        constexpr float epsilon = 1e-3;
        float sum = 0;
        for (int i = 0; i < 9; ++i)
        {
            sum += kernel[i];
        }
        if (sum >= -epsilon && sum <= epsilon)
            return kernel;
        for (int i = 0; i < 9; ++i)
        {
            kernel[i] /= sum;
        }
        return kernel;
    }

    void kernel(bmp::BmpView img, Kernel kernel)
    {
        static const int dirX[] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
        static const int dirY[] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};

        const bmp::Bmp imgCpy = img;

        auto inImage = [&imgCpy](int row, int col) -> bool
        {
            return row >= 0 && row < imgCpy.height() && col >= 0 && col < imgCpy.width();
        };

        for (int row = 0; row < img.height(); ++row)
        {
            for (int col = 0; col < img.width(); ++col)
            {
                float red = 0, green = 0, blue = 0;
                for (int i = 0; i < 9; ++i)
                {
                    bmp::RGB pixel = imgCpy(row, col);
                    if (inImage(row + dirX[i], col + dirY[i]))
                    {
                        pixel = imgCpy(row + dirX[i], col + dirY[i]);
                    }
                    red += kernel[i] * pixel.red;
                    green += kernel[i] * pixel.green;
                    blue += kernel[i] * pixel.blue;
                }

                auto &pixel = img(row, col);
                pixel.red = std::min(255, std::max<int>(0, red));
                pixel.green = std::min(255, std::max<int>(0, green));
                pixel.blue = std::min(255, std::max<int>(0, blue));
            }
        }
    }

    void sharpen(bmp::BmpView img)
    {
        static const Kernel sharpen = {0, -1, 0, -1, 5, -1, 0, -1, 0};
        kernel(img, sharpen);
    }

    void sepia(bmp::BmpView img)
    {
        for (int row = 0; row < img.height(); ++row)
        {
            for (int col = 0; col < img.width(); ++col)
            {
                auto &pixel = img(row, col);
                auto tmp = pixel;
                pixel.red = std::min<int>(255, (0.393 * tmp.red) + (0.769 * tmp.green) + (0.189 * tmp.blue));
                pixel.green = std::min<int>(255, (0.349 * tmp.red) + (0.686 * tmp.green) + (0.168 * tmp.blue));
                pixel.blue = std::min<int>(255, (0.272 * tmp.red) + (0.534 * tmp.green) + (0.131 * tmp.blue));
            }
        }
    }

    void drawline(bmp::BmpView img, Point p1, Point p2, bmp::RGB color)
    {
        int dx = p2.x - p1.x;
        int dy = p2.y - p1.y;
        int steps = std::max(std::abs(dx), std::abs(dy));

        float xinc = dx / static_cast<float>(steps);
        float yinc = dy / static_cast<float>(steps);

        float x = p1.x;
        float y = p1.y;

        for (int i = 0; i <= steps; ++i)
        {
            img(y, x) = color;
            x += xinc;
            y += yinc;
        }
    }

    void xshape(bmp::BmpView img, bmp::RGB color)
    {
        const Point top_left = {0, 0};
        const Point top_right = {img.width(), 0};
        const Point bottom_left = {1, img.height()};
        const Point bottom_right = {img.width() - 1, img.height()};
        drawline(img, top_left, bottom_right, color);
        drawline(img, top_right, bottom_left, color);
    }
}