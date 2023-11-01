#include "define.h"
#include "network.h"
#include "utils.h"

int initBroadcastSupplier(Supplier* supplier) {
    logInfo("Initializing broadcast for supplier.");
    int bcfd = initBroadcast(&supplier->bcast.addr);
    if (bcfd < 0) return bcfd;
    supplier->bcast.fd = bcfd;

    char names[MAX_TOTAL][BUF_NAME];
    int size;
    broadcastMe(supplier);
    char msg[BUF_MSG];
    recv(supplier->bcast.fd, msg, strlen(msg), 0);
    deserializer(supplier, msg, names, &size);

    if (!checkUnique(supplier->name, names, size)) {
        logError("Username not unique");
        exit(EXIT_FAILURE);
    }
    
    broadcast(supplier, serializer(supplier, REGISTERING));
    logInfo("Broadcast for supplier initialized.");
}

void initSupplier(Supplier* supplier, char* port) {
    logInfo("Initializing supplier.");
    initBroadcastSupplier(supplier);

    getInput(STDIN_FILENO, "Enter your name: ", supplier->name, BUF_NAME);

    supplier->tcpPort = atoi(port);
    initTCP(&supplier->tcpPort);

    logInfo("Supplier initialized.");
}

void broadcast(Supplier* supplier, char* msg) {
    sendto(supplier->bcast.fd, msg, strlen(msg), 0, 
           (struct sockaddr*)&supplier->bcast.addr, sizeof(supplier->bcast.addr));
}

char* serializer(Supplier* supplier, RegisteringState state) {
    char broadMsg[BUF_MSG] = {STRING_END};

    // clang-format off
    // state | name | tcpPort | type
    snprintf(broadMsg, BUF_MSG, "%d%c%s%c%d%c%d%c", 
             state, BCAST_IN_DELIM, 
             supplier->name, BCAST_IN_DELIM, 
             supplier->tcpPort, BCAST_IN_DELIM, 
             SUPPLIER, BCAST_OUT_DELIM);
    // clang-format on

    return strdup(broadMsg);
}

void broadcastMe(Supplier* supplier) {
    broadcast(supplier, serializer(supplier, NOT_REGISTERING));
}

void deserializer(Supplier* supplier, char* msg, char names[MAX_TOTAL][BUF_NAME], int* size) {
    char* part = strtok(msg, BCAST_OUT_DELIM);
    int shouldBroadcast = 0;
    while (part != NULL) {
        char* token = strtok(part, BCAST_IN_DELIM);
        if (token == NULL) break;
        RegisteringState state = atoi(token);
        token = strtok(NULL, BCAST_IN_DELIM);
        if (token == NULL) break;
        char* name = token;
        strcpy(names[(*size)++], name);
        token = strtok(NULL, BCAST_IN_DELIM);
        if (token == NULL) break;
        int tcpPort = atoi(token);
        token = strtok(NULL, BCAST_OUT_DELIM);
        if (token == NULL) break;
        BroadcastType type = atoi(token);

        shouldBroadcast |= (type == RESTAURANT && state == REGISTERING);
        shouldBroadcast |= (state == NOT_REGISTERING);

        part = strtok(NULL, BCAST_OUT_DELIM);
    }
    if (shouldBroadcast)
        broadcastMe(supplier);
}

void cli(Supplier* supplier, FdSet* fdset) { logError("No available commands."); }

void UDPHandler(Supplier* supplier, FdSet* fdset) {
    char msgBuf[BUF_MSG] = {STRING_END};
    int recvCount = recvfrom(supplier->bcast.fd, msgBuf, BUF_MSG, 0, NULL, NULL);
    if (recvCount == 0) {
        logError("Error receiving broadcast.");
        return;
    }
    char names[MAX_TOTAL][BUF_NAME];
    int size;
    deserializer(supplier, msgBuf, names, &size);
}

void newConnectionHandler(int fd, Supplier* supplier, FdSet* fdset) {
    logInfo("New connection request.");
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int newfd = accept(fd, (struct sockaddr*)&addr, &addrlen);
    if (newfd < 0) {
        logError("Error accepting new connection.");
        return;
    }
    FD_SETTER(newfd, fdset);
    logInfo("New connection accepted.");
}

void chatHandler(int fd, char* msgBuf, Supplier* supplier, FdSet* fdset) {
    int recvCount = recv(fd, msgBuf, BUF_MSG, 0);
    if (recvCount == 0) {
        logInfo("Connection closed.");
        close(fd);
        FD_CLRER(fd, fdset);
        return;
    }

    // name|quantity:port
    char* name = strtok(msgBuf, REQ_IN_DELIM);
    int quantity = atoi(strtok(NULL, REQ_DELIM));
    int port = atoi(strtok(NULL, REQ_DELIM));

    char msg[BUF_MSG] = {STRING_END};
    snprintf(msg, BUF_MSG, "You have a new request for %s", name);
    logMsg(msg);

    char ans[BUF_MSG];
    getInput(STDIN_FILENO, "Accept? (y/n)", ans, BUF_MSG);

    int ansFd = connectServer(port);

    if (!strcmp(ans, "y")) {
        snprintf(msg, BUF_MSG, "Request for %s accepted.", name);
        logInfo(msg);
        send(ansFd, ACCEPTED_MSG, strlen(ACCEPTED_MSG), 0);
    } else if (!strcmp(ans, "n")) {
        snprintf(msg, BUF_MSG, "Request for %s rejected.", name);
        logInfo(msg);
        send(ansFd, REJECTED_MSG, strlen(REJECTED_MSG), 0);
    } else {
        logError("Invalid answer.");
    }
}

void interface(Supplier* supplier) {
    char msgBuf[BUF_MSG] = {STRING_END};

    FdSet fdset;
    InitFdSet(&fdset, supplier->bcast.fd);

    while (1) {
        cliPrompt();
        memset(msgBuf, STRING_END, BUF_MSG);
        fdset.working = fdset.master;
        select(fdset.max + 1, &fdset.working, NULL, NULL, NULL);

        for (int i = 0; i <= fdset.max; ++i) {
            if (!FD_ISSET(i, &fdset.working)) continue;

            // this if is for having a clean interface when receiving messages
            if (i != STDIN_FILENO) write(STDOUT_FILENO, CLEAR_LINE_ANSI, CLEAR_LINE_LEN);

            if (i == STDIN_FILENO)
                cli(supplier, &fdset);
            else if (i == supplier->bcast.fd)
                UDPHandler(supplier, &fdset);
            else if (i == supplier->tcpPort)
                newConnectionHandler(i, supplier, &fdset);
            else
                chatHandler(i, msgBuf, supplier, &fdset);
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        logError("Usage: ./supplier <port>");
        exit(EXIT_FAILURE);
    }

    Supplier supplier;
    supplier.tcpPort = atoi(argv[1]);

    initSupplier(&supplier, argv[1]);

    interface(&supplier);
}