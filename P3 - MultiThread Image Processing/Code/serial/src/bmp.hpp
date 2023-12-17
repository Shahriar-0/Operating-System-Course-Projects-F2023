#ifndef BMP_HPP_INCLUDE
#define BMP_HPP_INCLUDE

#include <cstdint>
#include <string>
#include <vector>

namespace BMP24 {

#pragma pack(push, 1)
struct Header {
    uint16_t fileType;
    uint32_t fileSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
};
struct InfoHeader {
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

constexpr uint16_t FILE_TYPE = 0x4D42;

struct RGB {
    RGB() = default;
    RGB(uint8_t r, uint8_t g, uint8_t b);
    uint8_t blu;
    uint8_t grn;
    uint8_t red;
};

class BMP_View;

class BMP {
public:
    BMP() = default;
    BMP(int width, int height);
    BMP(BMP_View view);
    ~BMP();
    BMP(const BMP& other);
    BMP(BMP&& other) noexcept;
    BMP& operator=(const BMP& rhs);
    BMP& operator=(BMP&& rhs) noexcept;
    friend void swap(BMP& a, BMP& b);

    enum class ReadResult {
        success,
        open_err,
        invalid_file,
        unsupported_bmp,
        alloc_err,
    };

    bool create(int width, int height);
    ReadResult read(const std::string& filename);
    bool write(const std::string& filename);

    int width() const;
    int height() const;
    bool valid() const;

    RGB& operator()(int row, int col);
    const RGB& operator()(int row, int col) const;

private:
    char* data_ = nullptr;
    bool valid_ = false;
    Header hdr_;
    InfoHeader infoHdr_;
    int padding_;
    std::vector<std::vector<RGB>> datavec_;

    static int calcPadding(int width);
    RGB& getPixel(int row, int col);

    void dataToVector();
    void dataFromVector();
};

class BMP_View {
public:
    BMP_View() = default;
    BMP_View(BMP& bmp);
    BMP_View(BMP& bmp, int row, int col, int width, int height);
    BMP_View& operator=(BMP& rhs);
    ~BMP_View() = default;

    int width() const;
    int height() const;

    RGB operator()(int row, int col) const;
    RGB& operator()(int row, int col);

    static void replace(BMP_View subBmp, BMP_View src);

private:
    BMP* bmp_ = nullptr;
    int row_;
    int col_;
    int width_;
    int height_;
};

}  // namespace BMP24

#endif  // BMP_HPP_INCLUDE
