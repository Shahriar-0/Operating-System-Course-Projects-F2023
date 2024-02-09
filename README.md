# Operating-System-Course-Projects-F2023

- [Operating-System-Course-Projects-F2023](#operating-system-course-projects-f2023)
  - [CA1: Socket Programming](#ca1-socket-programming)
  - [CA2: MapReduce Counter](#ca2-mapreduce-counter)
  - [CA3: Parallel Image Processing](#ca3-parallel-image-processing)

## CA1: Socket Programming

A customer-restaurant-supplier program is implemented in C using the POSIX sockets API. Internally, messaging is implemented using TCP sockets.
There can be many processes, and each take a port to listen for UDP broadcasts of product changes:

```text
./customer <PORT>
./restaurant <PORT>
./supplier <PORT>
```

Each process has a CLI to interact with the program. You can use the `help` command to see the available commands.
Each of them store appropriate logs in a file named after the process.

## CA2: MapReduce Counter

A MapReduce model is used to count the cost of facilities in each genre listed in CSV files. Each building has its own CSV file, that stores the cost of each bill for each month.

```text
./company_processor.out <bills.csv>
```

There are multiple part files, each containing a part of the data. Each building creates it's own process to process the part file, and each resource is managed by a separate process, all these processes communicate using POSIX unnamed pipes. There is a main financial process which is connected to the other processes using POSIX named pipes.

## CA3: Parallel Image Processing

Image filters and effects are applied to a *BMP24* image file both serially, and in parallel using POSIX threads.  
The program take an image as input, reads it using the fully implemented `Bmp` class *(bmp.hpp)*, and applies the wanted filters which are implemented in the `filter` namespace *(filter.hpp)*.

```text
./ImageFilters.out <IMAGE FILE>
```

Each filter takes a `BmpView`, which is a custom view of the original `Bmp` (this can be the whole image as well which is done implicitly), and changes the original image pixels.  

In the example *main.cpp*, three filters (vertical flip, Gaussian blur, and purple haze) are applied and the result is written in `output.bmp`.

For the parallel version, a thread pool is implemented using pthreads.  
Tasks are added to a mutex protected queue which threads execute.  
In the example *main.cpp*, the image is split into 8 BmpViews (one for each thread) and the filter tasks are ran by the threads concurrently.
