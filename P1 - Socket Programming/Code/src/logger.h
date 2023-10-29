#ifndef LOGGER_H_INCLUDE
#define LOGGER_H_INCLUDE

#include "ansi_colors.h"
#include "define.h"

void logNormal(const char* msg);
void logInput(const char* msg);
void logMsg(const char* msg);
void logInfo(const char* msg);
void logWarning(const char* msg);
void logError(const char* msg);

void logBrightRed(const char* msg);
void logBrightGreen(const char* msg);
void logBrightYellow(const char* msg);
void logBrightBlue(const char* msg);
void logBrightMagenta(const char* msg);
void logBrightCyan(const char* msg);
void logBrightWhite(const char* msg);

void logBackgroundRed(const char* msg);
void logBackgroundGreen(const char* msg);
void logBackgroundYellow(const char* msg);
void logBackgroundBlue(const char* msg);
void logBackgroundMagenta(const char* msg);
void logBackgroundCyan(const char* msg);
void logBackgroundWhite(const char* msg);

#endif  // LOGGER_H_INCLUDE
