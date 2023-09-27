#ifndef BMP_HPP
#define BMP_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace bmp
{
    constexpr uint16_t BMP_SIGNATURE = 0x4D42; // "BM"

#pragma pack(push, 1)
    struct Header
    {
        uint16_t fileType;
        uint32_t fileSize;
        uint16_t reserved1;
        uint16_t reserved2;
        uint32_t offset;
    };
    struct InfoHeader
    {
        uint32_t headerSize;
        int32_t width;
        int32_t height;
        uint16_t planes;
        uint16_t bitCount;
        uint32_t compression;
        uint32_t imageSize;
        int32_t xPixelsPerMeter;
        int32_t yPixelsPerMeter;
        uint32_t colorsUsed;
        uint32_t colorsImportant;
    };
#pragma pack(pop)

    struct RGB
    {
        RGB() = default;
        RGB(uint8_t r, uint8_t g, uint8_t b);
        uint8_t blue;
        uint8_t green;
        uint8_t red;
    };

    class BmpView;

    class Bmp
    {
    public:
        Bmp() = default;
        Bmp(int width, int height);
        Bmp(BmpView view);
        ~Bmp();
        Bmp(const Bmp &other);
        Bmp(Bmp &&other) noexcept;
        Bmp &operator=(const Bmp &rhs);
        Bmp &operator=(Bmp &&rhs) noexcept;
        friend void swap(Bmp &a, Bmp &b);

        enum class ReadResult
        {
            success,
            open_err,
            invalid_file,
            unsupported_bmp,
            alloc_err,
        };

        bool create(int width, int height);
        ReadResult read(const std::string &filename);
        bool write(const std::string &filename);

        int width() const;
        int height() const;
        bool valid() const;

        RGB &operator()(int row, int col);
        const RGB &operator()(int row, int col) const;

    private:
        char *data_ = nullptr;
        bool valid_ = false;
        Header hdr_;
        InfoHeader infoHdr_;
        int padding_;
        std::vector<std::vector<RGB>> datavec_;

        static int calcPadding(int width);
        RGB &getPixel(int row, int col);

        void dataToVector();
        void dataFromVector();
    };

    class BmpView
    {
    public:
        BmpView() = default;
        BmpView(Bmp &bmp);
        BmpView(Bmp &bmp, int row, int col, int width, int height);
        BmpView &operator=(Bmp &rhs);
        ~BmpView() = default;

        int width() const;
        int height() const;

        RGB operator()(int row, int col) const;
        RGB &operator()(int row, int col);

        static void replace(BmpView subBmp, BmpView src);

    private:
        Bmp *bmp_ = nullptr;
        int row_;
        int col_;
        int width_;
        int height_;
    };

}; // namespace bmp

#endif // BMP_HPP