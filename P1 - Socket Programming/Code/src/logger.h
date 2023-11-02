#ifndef LOGGER_H_INCLUDE
#define LOGGER_H_INCLUDE

#include "ansi_colors.h"
#include "define.h"

// Write txt to filename. Returns 1 on error, 0 on success.
int writeToFile(const char* filename, const char* ext, const char* txt);

void logLamination();

void logNormal(const char* msg, char* name);
void logInput(const char* msg, char* name);
void logMsg(const char* msg, char* name);
void logInfo(const char* msg, char* name);
void logWarning(const char* msg, char* name);
void logError(const char* msg, char* name);

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
