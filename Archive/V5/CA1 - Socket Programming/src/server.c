#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "logger.h"
#include "types.h"
#include "utils.h"

int setupServer(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    int optval = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        exit(1);
    }
    return server_fd;
}

int acceptClient(int server_fd) {
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int client_fd = accept(server_fd, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (client_fd < 0) {
        perror("accept");
        exit(1);
    }
    return client_fd;
}

int getClientType(ClientArray clients, int id) {
    for (int i = 0; i < clients.size; i++) {
        if (clients.ptr[i].id == id)
            return clients.ptr[i].type;
    }
    return -1;
}

int main(int argc, char const* argv[]) {
    int server_fd, new_socket, max_sd;
    srand(time(NULL));

    char buffer[1024] = {0};
    char msgBuf[BUF_MSG] = {'\0'};

    if (argc != 2) {
        logError("Usage: ./server <port>");
        exit(1);
    }

    fd_set master_set, working_set;

    ClientArray clients;
    memset(&clients, 0, sizeof(clients));
    QuestionArray questions;
    memset(&questions, 0, sizeof(questions));
    questions.last = 0;

    PortArray ports;
    memset(&ports, 0, sizeof(ports));

    server_fd = setupServer(atoi(argv[1]));

    FD_ZERO(&master_set);
    max_sd = server_fd;
    FD_SET(server_fd, &master_set);

    write(1, "Server is running\n", 18);

    while (1) {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);

        for (int i = 0; i <= max_sd; i++) {
            if (FD_ISSET(i, &working_set)) {
                if (i == server_fd) { // new clinet
                    new_socket = acceptClient(server_fd);
                    FD_SET(new_socket, &master_set);
                    if (new_socket > max_sd)
                        max_sd = new_socket;
                    snprintf(buffer, 1024, "New client connected. fd = %d", new_socket);
                    logInfo(buffer);
                    snprintf(msgBuf, BUF_MSG, "$IDD$%d", new_socket);
                    send(new_socket, msgBuf, BUF_MSG, 0);
                }
                else { // client sending msg
                    int bytes_received;
                    memset(buffer, 0, 1024);
                    bytes_received = recv(i, buffer, 1024, 0);

                    if (bytes_received == 0) { // EOF
                        snprintf(buffer, 1024, "client fd = %d closed", i);
                        logInfo(buffer);
                        close(i);
                        FD_CLR(i, &master_set);
                        removeClient(&clients, i);
                        continue;
                    }

                    if (!strncmp(buffer, "$STU$", 5)) {
                        Client client;
                        client.id = i;
                        client.type = STUDENT;
                        addClient(&clients, client);
                    }
                    else if (!strncmp(buffer, "$TAA$", 5)) {
                        Client client;
                        client.id = i;
                        client.type = TA;
                        addClient(&clients, client);
                    }
                    else if (!strncmp(buffer, "$CLS$", 5)) {
                        int q_id;
                        char* q_id_str = strtok(buffer + 5, "$");
                        strToInt(q_id_str, &q_id);
                        char* aMsg = strtok(NULL, "$");

                        Question* q = getQuestion(&questions, q_id);
                        strcpy(q->aMsg, aMsg);
                        q->type = ANSWERED;
                        saveQuestion(q);
                    }
                    else if (!strncmp(buffer, "$REQ$", 5)) {
                        int port;
                        char* prt_str = strtok(buffer + 5, "$");
                        strToInt(prt_str, &port);
                        Question* q = getQuestionByPort(&questions, port);
                        if (q != NULL && q->type == DISCUSSING) {
                            snprintf(msgBuf, BUF_MSG, "$ACC$%d$%d$%d", port, q->author, q->ta);
                            send(i, msgBuf, strlen(msgBuf), 0);
                        }
                        else {
                            snprintf(msgBuf, BUF_MSG, "$PRM$");
                            send(i, msgBuf, strlen(msgBuf), 0);
                        }
                    }
                    else if (!strncmp(buffer, "$SUS$", 5)) {
                        logInfo(buffer);
                        int q_id;
                        char* q_str = strtok(buffer + 5, "$");
                        strToInt(q_str, &q_id);
                        logInfo(q_str);
                        Question* q = getQuestion(&questions, q_id);
                        if (q != NULL) {
                            q->type = WAITING;
                            q->ta = -1;
                        }
                        else {
                            logError("Question not found");
                        }
                    }
                    else if (getClientType(clients, i) == STUDENT) {
                        if (!strncmp(buffer, "$ASK$", 5)) {
                            char* question = buffer + 5;
                            Question q;
                            q.id = questions.last++;
                            q.author = i;
                            q.type = WAITING;
                            strcpy(q.qMsg, question);
                            addQuestion(&questions, q);
                        }
                        else if (!strncmp(buffer, "$SSN$", 5)) {
                            sendDiscussingQuestions(i, &questions);
                        }
                        else {
                            snprintf(msgBuf, BUF_MSG, "$PRM$");
                            send(i, msgBuf, strlen(msgBuf), 0);
                        }
                    }
                    else if (getClientType(clients, i) == TA) {
                        if (!strncmp(buffer, "$SQN$", 5)) {
                            sendWaitingQuestions(i, &questions);
                        }
                        else if (!strncmp(buffer, "$ANS$", 5)) {
                            int q_id, res;
                            char* answer = buffer + 5;
                            res = strToInt(answer, &q_id);
                            if (res == 1 || res == 2) {
                                send(i, "$PRM$", 5, 0);
                            }
                            else {
                                Question* q = getQuestion(&questions, q_id);
                                if (q == NULL || q->type != WAITING) {
                                    send(i, "$PRM$", 5, 0);
                                }
                                else {
                                    q->type = DISCUSSING;
                                    int port = generatePort(&ports);
                                    q->port = port;
                                    q->ta = i;

                                    snprintf(msgBuf, BUF_MSG, "$PRT$%d$%d", port, q->id);
                                    send(q->author, msgBuf, strlen(msgBuf), 0);
                                    send(i, msgBuf, strlen(msgBuf), 0);
                                }
                            }
                        }
                        else {
                            snprintf(msgBuf, BUF_MSG, "$PRM$");
                            send(i, msgBuf, strlen(msgBuf), 0);
                        }
                    }
                    else if (getClientType(clients, i) == -1) {
                        logInfo("4");
                    }
                }
            }
        }
    }

    return 0;
}