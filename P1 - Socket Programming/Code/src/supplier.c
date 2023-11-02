#include "define.h"
#include "network.h"
#include "utils.h"

void broadcast(Supplier* supplier, char* msg) {
    sendto(supplier->bcast.fd, msg, strlen(msg), 0, 
           (struct sockaddr*)&supplier->bcast.addr, sizeof(supplier->bcast.addr));
}

int initBroadcastSupplier(Supplier* supplier) {
    logInfo("Initializing broadcast for supplier.", supplier->name);
    int bcfd = initBroadcast(&supplier->bcast.addr);
    if (bcfd < 0) return bcfd;
    supplier->bcast.fd = bcfd;

    broadcast(supplier, REG_REQ_MSG);
    logInfo("Broadcast for supplier initialized.", supplier->name);
}

void initSupplier(Supplier* supplier, char* port) {
    logInfo("Initializing supplier.", supplier->name);
    initBroadcastSupplier(supplier);

    getInput(STDIN_FILENO, "Enter your name: ", supplier->name, BUF_NAME);

    supplier->tcpPort = atoi(port);
    initTCP(&supplier->tcpPort);

    logInfo("Supplier initialized.", supplier->name);
}


void broadcastMe(Supplier* supplier) { broadcast(supplier, serializerSupplier(supplier, NOT_REGISTERING)); }

void cli(Supplier* supplier, FdSet* fdset) { logError("No available commands.", supplier->name); }

void UDPHandler(Supplier* supplier, FdSet* fdset) {
    char msgBuf[BUF_MSG] = {STRING_END};
    int recvCount = recvfrom(supplier->bcast.fd, msgBuf, BUF_MSG, 0, NULL, NULL);
    if (recvCount == 0) {
        logError("Error receiving broadcast.", supplier->name);
        return;
    }

    if (!strcmp(msgBuf, REG_REQ_MSG))
        broadcast(supplier, serializerSupplier(supplier, REGISTERING));
    else {
        char* name;
        int port;
        BroadcastType type;
        deserializer(msgBuf, &name, &port, &type);
        if (!strcmp(supplier->name, name) && supplier->tcpPort != port) {
            int fd = connectServer(port);
            send(fd, TERMINATE_MSG, strlen(TERMINATE_MSG), 0);
            return;
        }
    }
}

void newConnectionHandler(int fd, Supplier* supplier, FdSet* fdset) {
    logInfo("New connection request.", supplier->name);
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int newfd = accept(fd, (struct sockaddr*)&addr, &addrlen);
    if (newfd < 0) {
        logError("Error accepting new connection.", supplier->name);
        return;
    }
    FD_SETTER(newfd, fdset);
    logInfo("New connection accepted.", supplier->name);
}

void chatHandler(int fd, char* msgBuf, Supplier* supplier, FdSet* fdset) {
    int recvCount = recv(fd, msgBuf, BUF_MSG, 0);
    if (recvCount == 0) {
        logInfo("Connection closed.", supplier->name);
        close(fd);
        FD_CLRER(fd, fdset);
        return;
    }

    // name|quantity:port
    // terminate msg
    char* name = strtok(msgBuf, REQ_IN_DELIM);

    if (!strcmp(name, TERMINATE_MSG)) {
        logError("Duplication in username", supplier->name);
        exit(EXIT_FAILURE);
    }

    int quantity = atoi(strtok(NULL, REQ_DELIM));
    int port = atoi(strtok(NULL, REQ_DELIM));

    char msg[BUF_MSG] = {STRING_END};
    snprintf(msg, BUF_MSG, "You have a new request for %s", name);
    logMsg(msg);

    yesNoPromptSupplier(name, port);
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
        perror("Usage: ./supplier <port>");
        exit(EXIT_FAILURE);
    }

    Supplier supplier;
    supplier.tcpPort = atoi(argv[1]);

    initSupplier(&supplier, argv[1]);

    interface(&supplier);
}