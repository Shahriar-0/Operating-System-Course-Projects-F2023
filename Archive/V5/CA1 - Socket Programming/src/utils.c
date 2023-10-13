#include "utils.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ansi_colors.h"
#include "logger.h"
#include "types.h"

void cliPrompt() {
    write(STDOUT_FILENO, ANSI_WHT ">> " ANSI_RST, 12);
}

void errnoPrint() {
    logError(strerror(errno));
}

int writeToFile(const char* filename, const char* ext, const char* txt) {
    char fname[BUF_NAME + 10] = {'\0'};
    strcpy(fname, filename);
    if (ext != NULL)
        strcat(fname, ext);

    chmod(fname, S_IWUSR | S_IRUSR);
    int fd = open(fname, O_CREAT | O_WRONLY | O_APPEND);
    if (fd < 0)
        return 1;

    if (write(fd, txt, strlen(txt)) < 0)
        return 1;
    close(fd);
    return 0;
}

void printNum(int fd, int num) {
    char buffer[12] = {'\0'};
    snprintf(buffer, 12, "%d", num);
    write(fd, buffer, strlen(buffer));
}

void getInput(int fd, const char* prompt, char* dst, size_t dstLen) {
    if (prompt != NULL)
        logInput(prompt);
    int cread = read(fd, dst, dstLen);
    if (cread <= 0) {
        errnoPrint();
        exit(EXIT_FAILURE);
    }
    dst[cread - 1] = '\0';
}

void addClient(ClientArray* arr, Client client) {
    if (arr->size == arr->capacity) {
        if (arr->capacity == 0)
            arr->capacity = 1;
        Client* arrNew = (Client*)realloc(arr->ptr, arr->capacity * 2 * sizeof(Client));
        if (arrNew == NULL) {
            logError("Allocation error.");
            exit(EXIT_FAILURE);
        }
        arr->ptr = arrNew;
        arr->capacity *= 2;
    }
    arr->ptr[arr->size] = client;
    ++arr->size;
}

void removeClient(ClientArray* arr, int id) {
    for (int i = 0; i < arr->size; i++) {
        if (arr->ptr[i].id == id) {
            arr->ptr[i] = arr->ptr[arr->size - 1];
            --arr->size;
            break;
        }
    }
}

void addQuestion(QuestionArray* arr, Question question) {
    if (arr->size == arr->capacity) {
        if (arr->capacity == 0)
            arr->capacity = 1;
        Question* arrNew = (Question*)realloc(arr->ptr, arr->capacity * 2 * sizeof(Question));
        if (arrNew == NULL) {
            logError("Allocation error.");
            exit(EXIT_FAILURE);
        }
        arr->ptr = arrNew;
        arr->capacity *= 2;
    }
    arr->ptr[arr->size] = question;
    ++arr->size;
}

void sendWaitingQuestions(int fd, QuestionArray* arr) {
    int empty = 1;
    for (int i = 0; i < arr->size; i++) {
        if (arr->ptr[i].type == WAITING) {
            char msgBuf[BUF_MSG];

            empty = 0;
            snprintf(msgBuf, BUF_MSG, "Q%d: %s\n", arr->ptr[i].id, arr->ptr[i].qMsg);
            write(fd, msgBuf, strlen(msgBuf));
        }
    }
    if (empty) {
        write(fd, "No questions waiting.\n", 22);
    }
}

int strToInt(const char* str, int* res) {
    char* end;
    long num = strtol(str, &end, 10);

    if (*end != '\0') return 1;
    if (errno == ERANGE) return 2;

    *res = num;
    return 0;
}

Question* getQuestion(QuestionArray* arr, int id) {
    for (int i = 0; i < arr->size; i++) {
        if (arr->ptr[i].id == id) {
            return &(arr->ptr[i]);
        }
    }
    return NULL;
}

// add port to ports array
void addPort(PortArray* arr, int port) {
    if (arr->size == arr->capacity) {
        if (arr->capacity == 0)
            arr->capacity = 1;
        int* arrNew = (int*)realloc(arr->ptr, arr->capacity * 2 * sizeof(int));
        if (arrNew == NULL) {
            logError("Allocation error.");
            exit(EXIT_FAILURE);
        }
        arr->ptr = arrNew;
        arr->capacity *= 2;
    }
    arr->ptr[arr->size] = port;
    ++arr->size;
}

int isExistingPort(PortArray* arr, int port) {
    for (int i = 0; i < arr->size; i++) {
        if (arr->ptr[i] == port) {
            return 1;
        }
    }
    return 0;
}

int generatePort(PortArray* ports) {
    int port = 0;
    while (port < 1024 || isExistingPort(ports, port)) {
        port = rand() % 65535;
    }
    return port;
}

void initBroadcastSocket(BroadcastInfo* br_info, int port) {
    int sock, broadcast = 1, opt = 1;
    struct sockaddr_in bc_address;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bc_address.sin_family = AF_INET;
    bc_address.sin_port = htons(port);
    bc_address.sin_addr.s_addr = inet_addr("172.30.143.255");

    bind(sock, (struct sockaddr*)&bc_address, sizeof(bc_address));
    br_info->fd = sock;
    br_info->addr = bc_address;
}

void saveQuestion(Question* q) {
    char buffer[1024] = {'\0'};
    snprintf(buffer, BUF_MSG, "Q%d: %s -> %s\n", q->id, q->qMsg, q->aMsg);
    writeToFile("questions", ".txt", buffer);
}

Question* getQuestionByPort(QuestionArray* arr, int port) {
    for (int i = 0; i < arr->size; i++) {
        if (arr->ptr[i].port == port) {
            return &arr->ptr[i];
        }
    }
    return NULL;
}

void sendDiscussingQuestions(int fd, QuestionArray* arr) {
    int empty = 1;

    for (int i = 0; i < arr->size; i++) {
        if (arr->ptr[i].type == DISCUSSING) {
            char msgBuf[BUF_MSG];
            empty = 0;
            snprintf(msgBuf, BUF_MSG, "Q%d: %s (Port: %d)\n", arr->ptr[i].id, arr->ptr[i].qMsg, arr->ptr[i].port);
            write(fd, msgBuf, strlen(msgBuf));
        }
    }
    if (empty) {
        write(fd, "No questions discussing.\n", 25);
    }
}
