#ifndef UTILS_H_INCLUDE
#define UTILS_H_INCLUDE

#include "types.h"

// Print the prompt string.
void cliPrompt();
// Print errno text representation to the standard error output.
void errnoPrint();

// Write txt to filename. Returns 1 on error, 0 on success.
int writeToFile(const char* filename, const char* ext, const char* txt);

// Print a number to file descriptor fd
void printNum(int fd, int num);
void getInput(int fd, const char* prompt, char* dst, size_t dstLen);

void addClient(ClientArray* arr, Client client);
void removeClient(ClientArray* arr, int id);
void addQuestion(QuestionArray* arr, Question question);
void sendWaitingQuestions(int fd, QuestionArray* arr);

int strToInt(const char* str, int* res);
Question* getQuestion(QuestionArray* arr, int id);

void addPort(PortArray* arr, int port);
int isExistingPort(PortArray* arr, int port);
int generatePort(PortArray* ports);
void initBroadcastSocket(BroadcastInfo* br_info, int port);
void saveQuestion(Question* q);
Question* getQuestionByPort(QuestionArray* arr, int port);
void sendDiscussingQuestions(int fd, QuestionArray* arr);

#endif