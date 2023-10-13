#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "logger.h"
#include "types.h"
#include "utils.h"

int my_id;
int timedOut = 0;
BroadcastInfo br_info;

int connectServer(int port) {
    int fd;
    struct sockaddr_in server_address;

    fd = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    if (connect(fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) { // checking for errors
        logError("Error in connecting to server");
        exit(1);
    }

    return fd;
}
ClientType getClientType(int server_fd) {
    logNormal("Which are you? (1-Student, 2-TA)");
    while (1) {
        char cmdBuf[BUF_CLI] = {'\0'};
        char msgBuf[BUF_MSG] = {'\0'};
        ClientType client_type;

        getInput(STDIN_FILENO, NULL, cmdBuf, BUF_CLI);

        if (strcmp(cmdBuf, "1") == 0) {
            client_type = STUDENT;
            snprintf(msgBuf, BUF_MSG, "$STU$");
            send(server_fd, msgBuf, strlen(msgBuf), 0);
            return client_type;
        }
        else if (strcmp(cmdBuf, "2") == 0) {
            client_type = TA;
            snprintf(msgBuf, BUF_MSG, "$TAA$");
            send(server_fd, msgBuf, strlen(msgBuf), 0);
            return client_type;
        }
        else {
            logError("Invalid input");
            continue;
        }
    }
}

void logHelp(ClientType client_type) {
    if (client_type == STUDENT)
        logNormal(
            "Available commands:\n"
            " - ask <question>: send a question to server.\n"
            " - show_sessions: show progressing sessions\n"
            " - connect <port>: connect to a TA.");
    else if (client_type == TA)
        logNormal(
            "Available commands:\n"
            " - show_questions: show list of all available questions.\n"
            " - answer <question_id>: choose a question to discuss about it.\n"
            " - connect <port>: connect to a student.");
}

void cli(fd_set* master_set, BroadcastInfo* br_info, int* max_sd, int server_fd, ClientType client_type) {
    char cmdBuf[BUF_CLI] = {'\0'};
    char msgBuf[BUF_MSG] = {'\0'};

    getInput(STDIN_FILENO, NULL, cmdBuf, BUF_CLI);
    char test[BUF_MSG] = {'\0'};

    if (br_info->fd != -1) {
        br_info->sending = 1;

        if (my_id == br_info->ta)
            alarm(0);

        if (strncmp(cmdBuf, "@close", 5) || my_id == br_info->host) {
            snprintf(msgBuf, BUF_MSG, "%d$%s", my_id, cmdBuf);
            sendto(br_info->fd, msgBuf, BUF_MSG, 0, (struct sockaddr*)&(br_info->addr), sizeof(br_info->addr));
        }
        return;
    }

    char* cmdPart = strtok(cmdBuf, " ");
    if (cmdPart == NULL)
        return;

    if (!strcmp(cmdPart, "help")) {
        logHelp(client_type);
    }
    else if (!strcmp(cmdPart, "ask")) {
        char* cmdPart = strtok(NULL, "");
        if (cmdPart == NULL) {
            logError("No question provided");
            return;
        }
        snprintf(msgBuf, BUF_MSG, "$ASK$%s", cmdPart);
        send(server_fd, msgBuf, strlen(msgBuf), 0);
    }
    else if (!strcmp(cmdPart, "show_sessions")) {
        snprintf(msgBuf, BUF_MSG, "$SSN$");
        send(server_fd, msgBuf, strlen(msgBuf), 0);
    }
    else if (!strcmp(cmdPart, "show_questions")) {
        snprintf(msgBuf, BUF_MSG, "$SQN$");
        send(server_fd, msgBuf, strlen(msgBuf), 0);
    }
    else if (!strcmp(cmdPart, "answer")) {
        char* cmdPart = strtok(NULL, " ");
        if (cmdPart == NULL) {
            logError("No answer provided");
            return;
        }

        snprintf(msgBuf, BUF_MSG, "$ANS$%s", cmdPart);
        send(server_fd, msgBuf, strlen(msgBuf), 0);
    }
    else if (!strcmp(cmdPart, "connect")) {
        char* cmdPart = strtok(NULL, " ");
        if (cmdPart == NULL) {
            logError("No port provided");
            return;
        }
        int port, res;
        res = strToInt(cmdPart, &port);
        if (res == 1 || res == 2) {
            logError("Invalid port number");
            return;
        }

        snprintf(msgBuf, BUF_MSG, "$REQ$%d", port);
        send(server_fd, msgBuf, BUF_MSG, 0);
    }
    else {
        logError("Invalid command.");
    }
}

void handleTimeout(int sig) {
    char buffer[1024] = {'\0'};
    snprintf(buffer, 1024, "%d$@tout", my_id);
    sendto(br_info.fd, buffer, 1024, 0, (struct sockaddr*)&(br_info.addr), sizeof(br_info.addr));
    timedOut = 1;
}

void timeOut(int br_fd, int server_fd, fd_set* master_set) {
    char buffer[1024] = {'\0'};
    snprintf(buffer, 1024, "$SUS$%d$", br_info.q_id);
    sendto(server_fd, buffer, 1024, 0, (struct sockaddr*)&(br_info.addr), sizeof(br_info.addr));
    close(br_fd);
    FD_CLR(br_fd, master_set);
    br_info.fd = -1;
    logWarning("Timeout (1min): Disconnected from discussion.");
    timedOut = 0;
}

int main(int argc, char const* argv[]) {
    int server_fd, new_socket, max_sd;

    if (argc != 2) {
        logError("Usage: ./server <port>");
        exit(1);
    }

    struct sigaction sigact = {.sa_handler = handleTimeout, .sa_flags = SA_RESTART};
    sigaction(SIGALRM, &sigact, NULL);

    char buffer[1024] = {0};
    char msgBuf[BUF_MSG] = {'\0'};

    memset(&br_info, 0, sizeof(br_info));
    br_info.fd = -1;
    br_info.q_id = -1;

    server_fd = connectServer(atoi(argv[1]));

    ClientType client_type = getClientType(server_fd);
    logHelp(client_type);

    fd_set master_set, working_set;

    FD_ZERO(&master_set);
    max_sd = server_fd;
    FD_SET(STDIN_FILENO, &master_set);
    FD_SET(server_fd, &master_set);

    while (1) {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);
        for (int i = 0; i <= max_sd; i++) {
            if (timedOut) {
                timeOut(br_info.fd, server_fd, &master_set);
            }
            if (FD_ISSET(i, &working_set)) {
                if (i != STDIN_FILENO) {
                    write(STDOUT_FILENO, "\x1B[2K\r", 5);
                }
                if (i == STDIN_FILENO) {
                    cli(&master_set, &br_info, &max_sd, server_fd, client_type);
                }
                else if (i == server_fd) {
                    int bytes_received;
                    memset(buffer, 0, 1024);
                    bytes_received = recv(i, buffer, 1024, 0);

                    if (bytes_received == 0) { // EOF
                        snprintf(buffer, 1024, "client fd = %d closed", i);
                        logInfo(buffer);
                        close(i);
                        FD_CLR(i, &master_set);
                        continue;
                    }

                    if (!strncmp(buffer, "$PRM$", 5)) {
                        logError("Permission Denied!");
                    }
                    else if (!strncmp(buffer, "$IDD$", 5)) {
                        char* id_str = strtok(buffer + 5, "$");
                        strToInt(id_str, &my_id);
                    }
                    else if (!strncmp(buffer, "$PRT$", 5)) {
                        int port, q_id;

                        char* port_str = strtok(buffer + 5, "$");
                        strToInt(port_str, &port);
                        char* q_id_str = strtok(NULL, "$");
                        strToInt(q_id_str, &q_id);

                        br_info.q_id = q_id;
                        snprintf(buffer, 1024, "port %d for question [%d]", port, q_id);
                        logNormal(buffer);
                    }
                    else if (!strncmp(buffer, "$ACC$", 5)) {
                        int port;

                        char* port_str = strtok(buffer + 5, "$");
                        strToInt(port_str, &port);
                        char* host_str = strtok(NULL, "$");
                        strToInt(host_str, &br_info.host);
                        char* ta_str = strtok(NULL, "$");
                        strToInt(ta_str, &br_info.ta);

                        initBroadcastSocket(&br_info, port);
                        logInfo("Connected to session.");

                        if (my_id == br_info.ta)
                            alarm(TIMEOUT);

                        FD_SET(br_info.fd, &master_set);

                        if (br_info.fd > max_sd)
                            max_sd = br_info.fd;
                    }
                    else {
                        logNormal(buffer);
                    }
                }
                else if (i == br_info.fd) {
                    int bytes_received, id;
                    memset(buffer, 0, 1024);
                    bytes_received = recv(i, buffer, 1024, 0);

                    char* id_str = strtok(buffer, "$");
                    strToInt(id_str, &id);
                    char* msg = strtok(NULL, "$");

                    if (!strncmp(msg, "@close", 6)) {
                        close(i);
                        FD_CLR(i, &master_set);
                        br_info.fd = -1;
                        logInfo("Disconnected from session");

                        char test[1024];

                        if (br_info.host == my_id) {
                            getInput(STDIN_FILENO, "Enter Answer of this question:\n", buffer, 1024);
                            memset(msgBuf, 0, BUF_MSG);
                            snprintf(msgBuf, BUF_MSG, "$CLS$%d$%s", br_info.q_id, buffer);
                            send(server_fd, msgBuf, BUF_MSG, 0);
                            logInfo("A & Q saved to the file.");
                        }
                    }
                    else if (!strncmp(msg, "@tout", 6)) {
                        timeOut(i, server_fd, &master_set);
                    }

                    else if (!br_info.sending) {
                        memset(msgBuf, 0, BUF_MSG);
                        snprintf(msgBuf, BUF_MSG, "User[%d]: %s", id, msg);
                        logNormal(msgBuf);
                    }
                    else
                        br_info.sending = 0;
                }
            }
        }
    }

    return 0;
}