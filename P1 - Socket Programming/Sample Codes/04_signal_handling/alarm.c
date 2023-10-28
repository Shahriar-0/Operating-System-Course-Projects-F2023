#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

void alarm_handler(int sig) {
    printf("tick tock\n");
}

int main(int argc, char const *argv[]) {
    char buff[1024];
    signal(SIGALRM, alarm_handler);
    siginterrupt(SIGALRM, 1);
    
    alarm(4);
    int read_ret = read(0, buff, 1024);
    alarm(0);
    printf("%d\n", read_ret);

    while (1);

    return 0;
}