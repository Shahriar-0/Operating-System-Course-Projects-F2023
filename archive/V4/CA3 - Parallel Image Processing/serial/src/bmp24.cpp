#include "bmp24.hpp"

#include <algorithm>
#include <fstream>

namespace bmp
{

    RGB::RGB(uint8_t r, uint8_t g, uint8_t b)
        : blue(b), green(g), red(r) {}

    Bmp::Bmp(int width, int height)
    {
        create(width, height);
    }

    Bmp::Bmp(BmpView view)
    {
        create(view.width(), view.height());
        for (int row = 0; row < view.height(); ++row)
        {
            for (int col = 0; col < view.width(); ++col)
            {
                (*this)(row, col) = view(row, col);
            }
        }
    }

    Bmp::~Bmp()
    {
        delete[] data_;
    }

    Bmp::Bmp(const Bmp &other)
    {
        if (!other.valid_)
            return;
        delete[] data_;
        data_ = new char[other.hdr_.fileSize];
        std::copy(other.data_, other.data_ + other.hdr_.fileSize, data_);
        valid_ = true;
        hdr_ = other.hdr_;
        infoHdr_ = other.infoHdr_;
        padding_ = other.padding_;
        datavec_ = other.datavec_;
    }

    Bmp::Bmp(Bmp &&other) noexcept
    {
        swap(*this, other);
    }

    Bmp &Bmp::operator=(const Bmp &rhs)
    {
        Bmp temp(rhs);
        swap(*this, temp);
        return *this;
    }

    Bmp &Bmp::operator=(Bmp &&rhs) noexcept
    {
        swap(*this, rhs);
        return *this;
    }

    void swap(Bmp &a, Bmp &b)
    {
        using std::swap;
        swap(a.data_, b.data_);
        swap(a.valid_, b.valid_);
        swap(a.hdr_, b.hdr_);
        swap(a.infoHdr_, b.infoHdr_);
        swap(a.padding_, b.padding_);
        swap(a.datavec_, b.datavec_);
    }

    bool Bmp::create(int width, int height)
    {
        valid_ = false;
        padding_ = calcPadding(width);

        hdr_.fileType = BMP_SIGNATURE;
        hdr_.fileSize = sizeof(Header) + sizeof(InfoHeader) + (width + padding_) * height * sizeof(RGB);
        hdr_.reserved1 = 0;
        hdr_.reserved2 = 0;
        hdr_.offset = sizeof(Header) + sizeof(InfoHeader);

        delete[] data_;
        data_ = new (std::nothrow) char[hdr_.fileSize];
        if (data_ == nullptr)
            return false;
        std::fill(data_, data_ + hdr_.fileSize, 0x00);

        infoHdr_ = {0};
        infoHdr_.headerSize = sizeof(InfoHeader);
        infoHdr_.width = width;
        infoHdr_.height = height;
        infoHdr_.planes = 1;
        infoHdr_.bitCount = 24;
        infoHdr_.imageSize = (width + padding_) * height * sizeof(RGB);

        std::copy((char *)&hdr_, (char *)&hdr_ + sizeof(Header), data_);
        std::copy((char *)&infoHdr_, (char *)&infoHdr_ + sizeof(InfoHeader), data_ + sizeof(Header));

        valid_ = true;
        datavec_.assign(height, std::vector<RGB>(width));
        return true;
    }

    Bmp::ReadResult Bmp::read(const std::string &filename)
    {
        std::ifstream file(filename, std::ios_base::binary);
        if (!file.is_open())
            return ReadResult::open_err;
        valid_ = false;

        file.read((char *)&hdr_, sizeof(Header));
        file.read((char *)&infoHdr_, sizeof(InfoHeader));

        if (hdr_.fileType != BMP_SIGNATURE)
            return ReadResult::invalid_file;
        if (infoHdr_.bitCount != 24 || infoHdr_.compression != 0)
            return ReadResult::unsupported_bmp;
        padding_ = calcPadding(infoHdr_.width);

        file.seekg(0, std::ios_base::end);
        std::streampos length = file.tellg();
        file.seekg(0, std::ios_base::beg);

        delete[] data_;
        data_ = new (std::nothrow) char[length];
        if (data_ == nullptr)
            return ReadResult::alloc_err;
        file.read(data_, length);

        valid_ = true;
        dataToVector();
        return ReadResult::success;
    }

    bool Bmp::write(const std::string &filename)
    {
        if (!valid_)
            return false;
        dataFromVector();

        std::ofstream file(filename, std::ios_base::binary);
        if (!file.is_open())
            return false;

        file.write(data_, hdr_.fileSize);
        return true;
    }

    int Bmp::width() const { return infoHdr_.width; }
    int Bmp::height() const { return infoHdr_.height; }
    bool Bmp::valid() const { return valid_; }

    RGB &Bmp::operator()(int row, int col)
    {
        return datavec_[row][col];
    }
    const RGB &Bmp::operator()(int row, int col) const
    {
        return datavec_[row][col];
    }

    int Bmp::calcPadding(int width)
    {
        const int bytesInRow = width * sizeof(RGB);
        return bytesInRow % 4 ? 4 - bytesInRow % 4 : 0;
    }

    RGB &Bmp::getPixel(int row, int col)
    {
        const int rowStart = (height() - 1 - row) * (width() * sizeof(RGB) + padding_);
        char *const pixelPos = data_ + hdr_.offset + rowStart + col * sizeof(RGB);
        return *(RGB *)pixelPos;
    }

    void Bmp::dataToVector()
    {
        datavec_.assign(height(), std::vector<RGB>(width()));
        for (int row = 0; row < height(); ++row)
        {
            for (int col = 0; col < width(); ++col)
            {
                datavec_[row][col] = getPixel(row, col);
            }
        }
    }

    void Bmp::dataFromVector()
    {
        for (int row = 0; row < height(); ++row)
        {
            for (int col = 0; col < width(); ++col)
            {
                getPixel(row, col) = datavec_[row][col];
            }
        }
    }

    // BmpView

    BmpView::BmpView(Bmp &bmp)
        : bmp_(&bmp), row_(0), col_(0), width_(bmp.width()), height_(bmp.height()) {}

    BmpView::BmpView(Bmp &bmp, int row, int col, int width, int height)
        : bmp_(&bmp), row_(row), col_(col), width_(width), height_(height) {}

    BmpView &BmpView::operator=(Bmp &other)
    {
        bmp_ = &other;
        row_ = 0;
        col_ = 0;
        width_ = other.width();
        height_ = other.height();
        return *this;
    }

    int BmpView::width() const { return width_; }
    int BmpView::height() const { return height_; }

    RGB BmpView::operator()(int row, int col) const
    {
        return (*bmp_)(row_ + row, col_ + col);
    }

    RGB &BmpView::operator()(int row, int col)
    {
        return (*bmp_)(row_ + row, col_ + col);
    }

    void BmpView::replace(BmpView subBmp, BmpView src)
    {
        for (int row = 0; row < subBmp.height(); ++row)
        {
            for (int col = 0; col < subBmp.width(); ++col)
            {
                subBmp(row, col) = src(row, col);
            }
        }
    }

} // namespace bmp