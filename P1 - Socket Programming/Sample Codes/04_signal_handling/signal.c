#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>


void handle_signal(int signal) {
    printf("signal %d received!\n", signal);
    printf("quiting...\n");
    exit(EXIT_SUCCESS);   
}

int main(int argc, char const *argv[]) {
    signal(SIGINT, handle_signal);

    while (1);

    return 0;
}