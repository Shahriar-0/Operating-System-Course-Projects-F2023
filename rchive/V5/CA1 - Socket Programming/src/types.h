#ifndef TYPES_H
#define TYPES_H

#include <netinet/in.h>
#include <sys/select.h>

#define BUF_NAME  32
#define BUF_PNAME 64
#define BUF_CLI   128
#define BUF_MSG   512
#define BCAST_IP  "192.168.1.255"
#define TIMEOUT   60

typedef enum {
    STUDENT = 0,
    TA = 1
} ClientType;

typedef enum {
    WAITING = 0,
    DISCUSSING = 1,
    ANSWERED = 2
} QuestionType;

typedef struct
{
    int id;
    ClientType type;
} Client;

typedef struct
{
    int fd;
    struct sockaddr_in addr;
    int sending;
    int q_id;
    int host;
    int ta;
} BroadcastInfo;

typedef struct
{
    int id;
    int author;
    int ta;
    QuestionType type;
    char qMsg[BUF_MSG];
    char aMsg[BUF_MSG];
    int port;
} Question;

typedef struct
{
    Question* ptr;
    int size;
    int capacity;
    int last;
} QuestionArray;

typedef struct {
    Client* ptr;
    int size;
    int capacity;
} ClientArray;

typedef struct {
    int* ptr;
    int size;
    int capacity;
} PortArray;

#endif