#ifndef UTILS_H_INCLUDE
#define UTILS_H_INCLUDE

#include "ansi_colors.h"
#include "define.h"
#include "logger.h"

// Print the prompt string.
void cliPrompt();
// Print a yes/no prompt to the standard output.
void yesNoPrompt(char* name, int quantity, unsigned short port, char* supplierName);
// Print errno text representation to the standard error output.
void errnoPrint();

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

// FdSet
void InitFdSet(FdSet* fdset, int UDPfd, int TCPfd);
void FD_SETTER(int socket, FdSet* fdset);
void FD_CLRER(int socket, FdSet* fdset);

// JSON
cJSON* loadJSON();

// check username uniqueness
int deserializer(char* msg, char** name, int* port, EXT* type);
int checkUnique(char* name, char names[MAX_TOTAL][BUF_NAME], int size);
char* serializerSupplier(Supplier* supplier, RegisteringState state);
char* serializerCustomer(Customer* customer, RegisteringState state);
char* serializerRestaurant(Restaurant* restaurant, RegisteringState state);
void exitall(char* name);
int isUniqueName(char* name);
void printWithType(int type);

#endif  // UTILS_H_INCLUDE