#ifndef UTILS_H_INCLUDE
#define UTILS_H_INCLUDE

#include "define.h"
#include "ansi_colors.h"
#include "logger.h"

// Print the prompt string.
void cliPrompt();
// Print errno text representation to the standard error output.
void errnoPrint();

// Write txt to filename. Returns 1 on error, 0 on success.
int writeToFile(const char* filename, const char* ext, const char* txt);

// Print a number to file descriptor fd
void printNum(int fd, int num);
void getInput(int fd, const char* prompt, char* dst, size_t dstLen);

// Return values:
// 0: success and res is set to the result.
// 1: str is not all numbers.
// 2: str number is out of bounds.
int strToInt(const char* str, int* res);
int strToPort(const char* str, unsigned short* res);
unsigned short strToPortErr(const char* str);

// JSON
void loadFoodNames(Customer* customer);
void loadMenu(Restaurant* restaurant);
cJSON* loadJSON()

#endif // UTILS_H_INCLUDE