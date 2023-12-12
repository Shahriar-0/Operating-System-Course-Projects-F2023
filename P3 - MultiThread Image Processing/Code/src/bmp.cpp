#include <fstream>
#include <iostream>

typedef int LONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;

#pragma pack(push, 1)
typedef struct tagBITMAPFILEHEADER {
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;
#pragma pack(pop)

int rows;
int cols;

bool fillAndAllocate(char*& buffer, const char* fileName, int& rows, int& cols, int& bufferSize) {
    std::ifstream file(fileName);
    if (!file) {
        std::cout << "File" << fileName << " doesn't exist!" << std::endl;
        return false;
    }

    file.seekg(0, std::ios::end);
    std::streampos length = file.tellg();
    file.seekg(0, std::ios::beg);

    buffer = new char[length];
    file.read(&buffer[0], length);

    PBITMAPFILEHEADER file_header;
    PBITMAPINFOHEADER info_header;

    file_header = (PBITMAPFILEHEADER)(&buffer[0]);
    info_header = (PBITMAPINFOHEADER)(&buffer[0] + sizeof(BITMAPFILEHEADER));
    rows = info_header->biHeight;
    cols = info_header->biWidth;
    bufferSize = file_header->bfSize;
    return true;
}

void getPixelsFromBMP24(int end, int rows, int cols, char* fileReadBuffer) {
    int count = 1;
    int extra = cols % 4;
    for (int i = 0; i < rows; i++) {
        count += extra;
        for (int j = cols - 1; j >= 0; j--) {
            for (int k = 0; k < 3; k++) {
                switch (k) {
                case 0:
                    // fileReadBuffer[end - count] is the red value
                    break;
                case 1:
                    // fileReadBuffer[end - count] is the green value
                    break;
                case 2:
                    // fileReadBuffer[end - count] is the blue value
                    break;
                }
                // go to the next position in the buffer
            }
        }
    }
}

void writeOutBmp24(char* fileBuffer, const char* nameOfFileToCreate, int bufferSize) {
    std::ofstream write(nameOfFileToCreate);
    if (!write) {
        std::cout << "Failed to write " << nameOfFileToCreate << std::endl;
        return;
    }

    int count = 1;
    int extra = cols % 4;
    for (int i = 0; i < rows; i++) {
        count += extra;
        for (int j = cols - 1; j >= 0; j--) {
            for (int k = 0; k < 3; k++) {
                switch (k) {
                case 0:
                    // write red value in fileBuffer[bufferSize - count]
                    break;
                case 1:
                    // write green value in fileBuffer[bufferSize - count]
                    break;
                case 2:
                    // write blue value in fileBuffer[bufferSize - count]
                    break;
                }
                // go to the next position in the buffer
            }
        }
    }
    write.write(fileBuffer, bufferSize);
}

int main(int argc, char* argv[]) {
    char* fileBuffer;
    int bufferSize;
    if (!fillAndAllocate(fileBuffer, argv[1], rows, cols, bufferSize)) {
        std::cout << "File read error" << std::endl;
        return 1;
    }

    // read input file
    // apply filters
    // write output file

    return 0;
}
