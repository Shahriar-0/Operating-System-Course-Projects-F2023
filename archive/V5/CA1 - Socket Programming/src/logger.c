#include "logger.h"

#include <string.h>
#include <unistd.h>

#include "ansi_colors.h"

void logNormal(const char* msg) {
    write(STDOUT_FILENO, msg, strlen(msg));
    write(STDOUT_FILENO, "\n", 1);
}

void logInput(const char* msg) {
    write(STDOUT_FILENO, ANSI_GRN "[Input] " ANSI_RST, 8 + ANSI_LEN);
    write(STDOUT_FILENO, msg, strlen(msg));
}

void logMsg(const char* msg) {
    write(STDOUT_FILENO, ANSI_MAG "[Message] " ANSI_RST, 10 + ANSI_LEN);
    write(STDOUT_FILENO, msg, strlen(msg));
    write(STDERR_FILENO, "\n", 1);
}

void logInfo(const char* msg) {
    write(STDOUT_FILENO, ANSI_BLU "[Info] " ANSI_RST, 7 + ANSI_LEN);
    write(STDOUT_FILENO, msg, strlen(msg));
    write(STDOUT_FILENO, "\n", 1);
}

void logWarning(const char* msg) {
    write(STDOUT_FILENO, ANSI_YEL "[Warning] " ANSI_RST, 10 + ANSI_LEN);
    write(STDOUT_FILENO, msg, strlen(msg));
    write(STDOUT_FILENO, "\n", 1);
}

void logError(const char* msg) {
    write(STDERR_FILENO, ANSI_RED "[Error] " ANSI_RST, 8 + ANSI_LEN);
    write(STDERR_FILENO, msg, strlen(msg));
    write(STDERR_FILENO, "\n", 1);
}
