#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <string.h>


// 0 -> stdin
// 1 -> stdout
// 2 -> stderr

int main(int argc, char const *argv[]) {
    char buff[1024];

    // read(0, buff, 1024);
    // printf("you said: %s\n", buff);

    // write(1, "writing without print!\n", 24);

    //* file read
    // memset(buff, 0, 1024);
    // int file_fd = open("file.txt", O_RDONLY);
    // read(file_fd, buff, 1024);
    // printf("file: %s\n", buff);
    // close(file_fd);

    //* file write
    int file_fd;
    file_fd = open("file.txt", O_APPEND | O_RDWR);
    strcpy(buff, "\nwriting to file!\n");
    write(file_fd, buff, strlen(buff));
    close(file_fd);
}