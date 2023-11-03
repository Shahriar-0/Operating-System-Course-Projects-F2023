#ifndef UTILS_H_INCLUDE
#define UTILS_H_INCLUDE

#include "ansi_colors.h"
#include "define.h"
#include "logger.h"

// Print the prompt string.
void cliPrompt();
// Print errno text representation to the standard error output.
void errnoPrint();

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