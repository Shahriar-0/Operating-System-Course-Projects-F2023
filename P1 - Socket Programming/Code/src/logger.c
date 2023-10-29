#include "logger.h"

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

void logBrightRed(const char* msg) {
    write(STDOUT_FILENO, ANSI_BRED, ANSI_LEN);
    write(STDOUT_FILENO, msg, strlen(msg));
    write(STDOUT_FILENO, ANSI_RST, ANSI_LEN);
    write(STDOUT_FILENO, "\n", 1);
}

void logBrightGreen(const char* msg) {
    write(STDOUT_FILENO, ANSI_BGRN, ANSI_LEN);
    write(STDOUT_FILENO, msg, strlen(msg));
    write(STDOUT_FILENO, ANSI_RST, ANSI_LEN);
    write(STDOUT_FILENO, "\n", 1);
}

void logBrightYellow(const char* msg) {
    write(STDOUT_FILENO, ANSI_BYEL, ANSI_LEN);
    write(STDOUT_FILENO, msg, strlen(msg));
    write(STDOUT_FILENO, ANSI_RST, ANSI_LEN);
    write(STDOUT_FILENO, "\n", 1);
}

void logBrightBlue(const char* msg) {
    write(STDOUT_FILENO, ANSI_BBLU, ANSI_LEN);
    write(STDOUT_FILENO, msg, strlen(msg));
    write(STDOUT_FILENO, ANSI_RST, ANSI_LEN);
    write(STDOUT_FILENO, "\n", 1);
}

void logBrightMagenta(const char* msg) {
    write(STDOUT_FILENO, ANSI_BMAG, ANSI_LEN);
    write(STDOUT_FILENO, msg, strlen(msg));
    write(STDOUT_FILENO, ANSI_RST, ANSI_LEN);
    write(STDOUT_FILENO, "\n", 1);
}

void logBrightCyan(const char* msg) {
    write(STDOUT_FILENO, ANSI_BCYN, ANSI_LEN);
    write(STDOUT_FILENO, msg, strlen(msg));
    write(STDOUT_FILENO, ANSI_RST, ANSI_LEN);
    write(STDOUT_FILENO, "\n", 1);
}

void logBrightWhite(const char* msg) {
    write(STDOUT_FILENO, ANSI_BWHT, ANSI_LEN);
    write(STDOUT_FILENO, msg, strlen(msg));
    write(STDOUT_FILENO, ANSI_RST, ANSI_LEN);
    write(STDOUT_FILENO, "\n", 1);
}

void logBackgroundRed(const char* msg) {
    write(STDOUT_FILENO, ANSI_BRED, ANSI_LEN);
    write(STDOUT_FILENO, msg, strlen(msg));
    write(STDOUT_FILENO, ANSI_RST, ANSI_LEN);
    write(STDOUT_FILENO, "\n", 1);
}

void logBackgroundGreen(const char* msg) {
    write(STDOUT_FILENO, ANSI_BGRN, ANSI_LEN);
    write(STDOUT_FILENO, msg, strlen(msg));
    write(STDOUT_FILENO, ANSI_RST, ANSI_LEN);
    write(STDOUT_FILENO, "\n", 1);
}

void logBackgroundYellow(const char* msg) {
    write(STDOUT_FILENO, ANSI_BYEL, ANSI_LEN);
    write(STDOUT_FILENO, msg, strlen(msg));
    write(STDOUT_FILENO, ANSI_RST, ANSI_LEN);
    write(STDOUT_FILENO, "\n", 1);
}

void logBackgroundBlue(const char* msg) {
    write(STDOUT_FILENO, ANSI_BBLU, ANSI_LEN);
    write(STDOUT_FILENO, msg, strlen(msg));
    write(STDOUT_FILENO, ANSI_RST, ANSI_LEN);
    write(STDOUT_FILENO, "\n", 1);
}

void logBackgroundMagenta(const char* msg) {
    write(STDOUT_FILENO, ANSI_BMAG, ANSI_LEN);
    write(STDOUT_FILENO, msg, strlen(msg));
    write(STDOUT_FILENO, ANSI_RST, ANSI_LEN);
    write(STDOUT_FILENO, "\n", 1);
}

void logBackgroundCyan(const char* msg) {
    write(STDOUT_FILENO, ANSI_BCYN, ANSI_LEN);
    write(STDOUT_FILENO, msg, strlen(msg));
    write(STDOUT_FILENO, ANSI_RST, ANSI_LEN);
    write(STDOUT_FILENO, "\n", 1);
}

void logBackgroundWhite(const char* msg) {
    write(STDOUT_FILENO, ANSI_BWHT, ANSI_LEN);
    write(STDOUT_FILENO, msg, strlen(msg));
    write(STDOUT_FILENO, ANSI_RST, ANSI_LEN);
    write(STDOUT_FILENO, "\n", 1);
}
