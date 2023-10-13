#include <iostream>
#include <fstream>
#include <vector>
#include <pthread.h>
#include <thread>

#define WHITE 255

using namespace std;

#pragma pack(1)

typedef int LONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;

typedef struct tagBITMAPFILEHEADER
{
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
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

typedef struct Pixel{
    int red;
    int green;
    int blue;
    Pixel(int r, int g, int b){
        red = max(0, min(255,r));
        green = max(0, min(255,g));
        blue = max(0, min(255,b));
    }
    Pixel(){
        red = 0;
        green = 0;
        blue = 0;
    }
} Pixel;

int Kernel[3][3] = {
        {0, -1, 0},
        {-1, 5, -1},
        {0, -1, 0}
};


struct Args{
    int col;
    int cols;
    int col2;
    int rows;
    int row;
    int row2;
    int extra;
    int xtr2;
    int threadcount;
    vector<vector<Pixel>>* image;
    vector<vector<Pixel>>* image2;
    char* buffer;
};

bool fillAndAllocate(char *&buffer, const char *fileName, int &rows, int &cols, int &bufferSize)
{
    std::ifstream file(fileName);

    if (file)
    {
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
        return 1;
    }
    else
    {
        cout << "File" << fileName << " doesn't exist!" << endl;
        return 0;
    }
}

void* bounded_getPixelsFromBMP24(void* arguments)
{
    struct Args *args;
    args = (struct Args *) arguments;
    int extra = args->xtr2; // col offset
    int idx = args->extra; // index
    int cols = args->cols;
    int row = args->row;
    int row2 = args->row2;
    vector<vector<Pixel>>* image = args->image;
    for (int i = row; i < row2; i++) {
        idx -= extra;
        for (int j = cols - 1; j >= 0; j--) {
            for (int k = 0; k < 3; k++) {
                idx--;
                auto c = (unsigned char) args->buffer[idx];
                switch (k) {
                    case 0:
                        (*image)[i][j].red = c;
                        break;
                    case 1:
                        (*image)[i][j].green = c;
                        break;
                    case 2:
                        (*image)[i][j].blue = c;
                        break;
                }

            }
        }
    }
    return nullptr;
}

vector<vector<Pixel>> getPixelsFromBMP24(int end, int& rows, int& cols, char *fileReadBuffer, int threadCount) {
    
    int index = end;
    int extra = cols % 4;
    vector<vector<Pixel>> picture(rows, vector<Pixel>(cols));
    int dx = (3*cols + extra) * rows;
    while(dx % threadCount != 0)threadCount--;
    int offset = end - dx, portion = rows / threadCount;
    pthread_t threads[threadCount];
    Args args[threadCount];
    int st = 0;

    for (int i = 0; i < threadCount; i++) {

        args[i].col = 0;
        args[i].cols = cols;
        args[i].rows = rows;
        args[i].row = st;
        st += portion;
        args[i].row2 = st;
        args[i].extra = index;
        args[i].xtr2 = extra;
        args[i].image = &picture;
        args[i].buffer = fileReadBuffer;

        pthread_create(&threads[i], nullptr, bounded_getPixelsFromBMP24, &args[i]);
        index -= (3*cols + extra) * portion;
    }

    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i], nullptr);
    }

    return picture;
}

void* bounded_writeOutBmp24(void* arguments) {
    struct Args *args;
    args = (struct Args *) arguments;
    int extra = args->xtr2; // col offset
    int idx = args->extra; // index
    int cols = args->cols;
    int row = args->row;
    int row2 = args->row2;
    vector<vector<Pixel>> *image = args->image;
    for (int i = row; i < row2; i++) {
        idx -= extra;
        for (int j = cols - 1; j >= 0; j--) {
            for (int k = 0; k < 3; k++) {
                idx--;
                switch (k) {
                    case 0:
                        args->buffer[idx] = char(args->image->at(i)[j].red);
                        break;
                    case 1:
                        args->buffer[idx] = char(args->image->at(i)[j].green);
                        break;
                    case 2:
                        args->buffer[idx] = char(args->image->at(i)[j].blue);
                        break;
                }
            }
        }
    }
    return nullptr;
}

void writeOutBmp24(char *fileBuffer, const char *nameOfFileToCreate, int bufferSize, int& rows, int& cols, vector<vector<Pixel>>& image, int threadCount) 
{
    std::ofstream write(nameOfFileToCreate);
    if (!write) {
        cout << "Failed to write " << nameOfFileToCreate << endl;
        return;
    }
    int index = bufferSize;
    int extra = cols % 4;
    int dx = (3*cols + extra) * rows;
    while(dx % threadCount != 0)threadCount--;
    int offset = bufferSize - dx, portion = rows / threadCount;
    pthread_t threads[threadCount];
    Args args[threadCount];
    int st = 0;

    for (int i = 0; i < threadCount; i++) {
        args[i].threadcount = threadCount;
        args[i].col = 0;
        args[i].cols = cols;
        args[i].rows = rows;
        args[i].row = st;
        st += portion;
        args[i].row2 = st;
        args[i].extra = index;
        args[i].xtr2 = extra;
        args[i].image = &image;
        args[i].buffer = fileBuffer;
        pthread_create(&threads[i], nullptr, bounded_writeOutBmp24, &args[i]);
        index -= (3*cols + extra) * portion;
    }

    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i], nullptr);
    }
    write.write(fileBuffer, bufferSize);
}

void* horizontal_mirror_column(void* arguments) 
{
    struct Args *args;
    args = (struct Args *) arguments;
    for (int row = args->row; row < args->row2; row++)
        for (int j = 0; j < args->col/2; j++) {
            Pixel temp = (*args->image)[row][j];
            (*args->image)[row][j] = (*args->image)[row][args->col - j - 1];
            (*args->image)[row][args->col - j - 1] = temp;
        }
    return nullptr;
}

void horizontal_mirror(vector<vector<Pixel>>& image, int& rows, int& cols, int thread_count) 
{
    auto *threads = new pthread_t[thread_count];
    int ratio = rows / thread_count;
    auto *args = new Args[thread_count];
    for (int i = 0; i < thread_count; i++) {
        args[i].image = &image;
        args[i].col = cols;
        int start = i * ratio;
        int end = (i + 1) * ratio;
        if (i == thread_count - 1) end = rows ;
        args[i].row = start, args[i].row2 = end;
        pthread_create(&threads[i], nullptr, reinterpret_cast<void *(*)(void *)>(horizontal_mirror_column),
                       (void *) &args[i]);
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], nullptr);
    }
    delete[] threads;
    delete[] args;
}

void* vertical_mirror_column(void* arguments) 
{        
    struct Args *args;
    args = (struct Args *) arguments;
    for (int i = 0; i < args->rows/2; i++)
        for (int col = args->col ; col < args->col2; col++) {
            Pixel temp = (*args->image)[i][col];
            (*args->image)[i][col] = (*args->image)[args->rows - i -1][col];
            (*args->image)[args->rows - i -1][col] = temp;
        }
    return nullptr;
}

void vertical_mirror(vector<vector<Pixel>>& image, int& rows, int& cols, int thread_count) 
{
    auto *threads = new pthread_t[thread_count];
    int ratio = cols / thread_count;
    auto *args = new Args[thread_count];
    for (int i = 0; i < thread_count; i++) {
        args[i].image = &image;
        args[i].rows = rows;
        int start = i * ratio;
        int end = (i + 1) * ratio;
        if (i == thread_count - 1) end = cols;
        args[i].col = start, args[i].col2 = end;
        pthread_create(&threads[i], nullptr, reinterpret_cast<void *(*)(void *)>(vertical_mirror_column),
                       (void *) &args[i]);
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], nullptr);
    }
    delete[] threads;
    delete[] args;
}

Pixel kernel_convolution(vector<vector<Pixel>>& image, int row, int col){
    int r=0, g=0, b=0;
    for(int i=-1; i<=1; i++){
        for(int j=-1; j<=1; j++){
            r += image[row+i][col+j].red * Kernel[i+1][j+1];
            g += image[row+i][col+j].green * Kernel[i+1][j+1];
            b += image[row+i][col+j].blue * Kernel[i+1][j+1];
        }
    }
    return {r, g, b};
}

void* bounded_convolution(void* arguments) {
    struct Args *args;
    args = (struct Args *) arguments;
    for (int row = args->row; row < args->row2; row++) {
        if(row == 0)
            continue;
        for (int j = 1; j < args->col - 1; j++) {
            (*args->image2)[row][j] = kernel_convolution(*args->image, row, j);
        }
    }
    return nullptr;
}

void edge_copy(vector<vector<Pixel>>& s_image, vector<vector<Pixel>>& r_image, int& rows, int& cols) {
    for(int t=0; t<rows; t++) {
        r_image[t][0] = s_image[t][0];
        r_image[t][cols-1] = s_image[t][cols-1];
    }
    for(int t=0; t<cols; t++) {
        r_image[0][t] = s_image[0][t];
        r_image[rows-1][t] = s_image[rows-1][t];
    }
}

void convolute_with_kernel(vector<vector<Pixel>>& image, int& rows, int& cols, int threads_count)
{
    vector<vector<Pixel>> temp(rows, vector<Pixel>(cols));
    auto *threads = new pthread_t[threads_count];
    int ratio = rows / threads_count;
    auto *args = new Args[threads_count];

    for (int i = 0; i < threads_count; i++) {
        args[i].image = &image;
        args[i].image2 = &temp;
        args[i].col = cols;
        int start = i * ratio;
        if (start == 0) start = 1;
        int end = (i + 1) * ratio;
        if (i == threads_count - 1) end = rows - 1;
        args[i].row = start, args[i].row2 = end;
        pthread_create(&threads[i], nullptr, reinterpret_cast<void *(*)(void *)>(bounded_convolution),
                       (void *) &args[i]);
    }

    for (int i = 0; i < threads_count; i++) {
        pthread_join(threads[i], nullptr);
    }
    image = temp;

    delete[] threads;
    delete[] args;
}

Pixel old_pixel(vector<vector<Pixel>>& image, int& row, int& col)
{
    Pixel temp;
    temp.red = (0.393 * image[row][col].red) + (0.769 * image[row][col].green) + (0.189 * image[row][col].blue);
    temp.green = (0.349 * image[row][col].red) + (0.686 * image[row][col].green) + (0.168 * image[row][col].blue);
    temp.blue = (0.272 * image[row][col].red) + (0.534 * image[row][col].green) + (0.131 * image[row][col].blue);

    if ( temp.red > WHITE)
        temp.red = WHITE;
    if ( temp.green > WHITE)
        temp.green = WHITE;
    if ( temp.blue > WHITE)
        temp.blue = WHITE;
    
    return temp;
}

void* bounded_Sepia_filter(void* arguments)
{
    struct Args *args;
    args = (struct Args *) arguments;
    for (int row = args->row; row < args->row2; row++) {
        for (int j = 0; j < args->cols; j++)
        {
            (*args->image)[row][j] = old_pixel(*args->image, row, j);
        }
    }
    return nullptr;
}

void Sepia_filter(vector<vector<Pixel>>& image, int& rows, int& cols, int threads_count)
{
    auto *threads = new pthread_t[threads_count];
    int ratio = rows / threads_count;
    auto *args = new Args[threads_count];

    for (int i = 0; i < threads_count; i++) {
        args[i].image = &image;
        args[i].cols = cols;
        int start = i * ratio;
        if (start == 0) start = 1;
        int end = (i + 1) * ratio;
        if (i == threads_count - 1) end = rows;
        args[i].row = start, args[i].row2 = end;
        pthread_create(&threads[i], nullptr, reinterpret_cast<void *(*)(void *)>(bounded_Sepia_filter),
                       (void *) &args[i]);
    }

    for (int i = 0; i < threads_count; i++) {
        pthread_join(threads[i], nullptr);
    }

    delete[] threads;
    delete[] args;
}
Pixel x_mark_pixel(vector<vector<Pixel>>& image, vector<vector<Pixel>>& image2,int& rows, int& cols )
{
    if ( image2[rows][cols].red != WHITE)
    {
        image2[rows][cols].red = image[rows][cols].red + image2[rows][cols].red;
        image2[rows][cols].green = image[rows][cols].green + image2[rows][cols].green;
        image2[rows][cols].blue = image[rows][cols].blue + image2[rows][cols].blue;
    }
    
    return image2[rows][cols];
}
void* bounded_x_mark_filter(void* arguments)
{
    struct Args *args;
    args = (struct Args *) arguments;
    for (int row = args->row; row < args->row2; row++) {
        for(int j=0; j < args->cols; j++)
        {
            (*args->image)[row][j] = x_mark_pixel(*args->image, *args->image2, row, j);
        }
    }
}
void x_mark_filter(vector<vector<Pixel>>& image, int& rows, int& cols, int threads_count)
{
    vector<vector<Pixel>> temp(rows, vector<Pixel>(cols));
    auto *threads = new pthread_t[threads_count];
    int ratio = rows / threads_count;
    auto *args = new Args[threads_count];

    for (int i = 0; i < cols ; i++)
    {
        int j = (rows * i)/cols;
        
        // first diameter
        temp[j][i].red = WHITE;
        temp[j][i].blue = WHITE;
        temp[j][i].green = WHITE;
        //second deimeter
        temp[rows-j -1][i].red = WHITE;
        temp[rows-j -1][i].blue = WHITE;
        temp[rows-j -1][i].green = WHITE;
    }

    for (int i = 0; i < threads_count; i++) {
        args[i].image = &image;
        args[i].cols = cols;
        args[i].image2 = &temp;
        int start = i * ratio;
        if (start == 0) start = 1;
        int end = (i + 1) * ratio;
        if (i == threads_count - 1) end = rows;
        args[i].row = start, args[i].row2 = end;
        pthread_create(&threads[i], nullptr, reinterpret_cast<void *(*)(void *)>(bounded_x_mark_filter),
                       (void *) &args[i]);
    }

    for (int i = 0; i < threads_count; i++) {
        pthread_join(threads[i], nullptr);
    }

    delete[] threads;
    delete[] args;

}


int main(int argc, char *argv[])
{
    char *fileBuffer;
    int bufferSize;
    int rows, cols;
    if (argc < 2)
    {
        cout << "Please provide an input file name" << endl;
        return 0;
    }
    char *fileName = argv[1];
    auto start_time = std::chrono::high_resolution_clock::now();
    if (!fillAndAllocate(fileBuffer, fileName, rows, cols, bufferSize))
    {
        cout << "File read error" << endl;
        return 1;
    }
    
    // read input file
    auto image = getPixelsFromBMP24(bufferSize, rows, cols, fileBuffer, 12);
    // apply filters
    ;
    horizontal_mirror(image, rows, cols, 12);
    vertical_mirror(image, rows, cols, 12);
    convolute_with_kernel(image, rows, cols, 12);
    Sepia_filter(image, rows, cols, 12);
    x_mark_filter(image, rows, cols, 12);

    // write output file
    writeOutBmp24(fileBuffer, "output.bmp", bufferSize, rows, cols, image,4);

    auto end_time = std::chrono::high_resolution_clock::now();
    cout << "Execution Time:" << std:: chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count()<< endl;
    
    pthread_exit(nullptr);
    
}