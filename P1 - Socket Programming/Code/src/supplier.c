#include "define.h"
#include "network.h"
#include "utils.h"

char* SupplierLogName(Supplier* supplier) {
    char* name[BUF_NAME];
    sprintf(name, "%s%s%d%s%d", supplier->name, NAME_DELIM, supplier->tcpPort, NAME_DELIM,
            SUPPLIER);
    return strdup(name);
}

void exiting(Supplier* supplier) { exitall(SupplierLogName(supplier)); }

int initBroadcastSupplier(Supplier* supplier) {
    logInfo("Initializing broadcast for supplier.", SupplierLogName(supplier));
    int bcfd = initBroadcast(&supplier->bcast.addr);
    if (bcfd < 0) return bcfd;
    supplier->bcast.fd = bcfd;

    logInfo("Broadcast for supplier initialized.", SupplierLogName(supplier));
}

void initSupplier(Supplier* supplier, char* port) {
    getInput(STDIN_FILENO, "Enter your name: ", supplier->name, BUF_NAME);
    if (!isUniqueName(supplier->name)) {
        write(1, "Name already taken.\n", strlen("Name already taken.\n"));
        exit(EXIT_FAILURE);
    }
    supplier->tcpPort = atoi(port);

    logInfo("Initializing supplier.", SupplierLogName(supplier));
    initBroadcastSupplier(supplier);

    supplier->tcpFd = initTCP(supplier->tcpPort);

    logInfo("Supplier initialized.", SupplierLogName(supplier));
}

void cli(Supplier* supplier, FdSet* fdset) {
    char msg[BUF_MSG] = {STRING_END};
    getInput(STDIN_FILENO, NULL, msg, BUF_MSG);

    if (!strcmp(msg, "exit"))
        exiting(supplier);
    else
        logError("Invalid command.", SupplierLogName(supplier));
}

void UDPHandler(Supplier* supplier, FdSet* fdset) {
    char msgBuf[BUF_MSG] = {STRING_END};
    int recvCount = recvfrom(supplier->bcast.fd, msgBuf, BUF_MSG, 0, NULL, NULL);
    if (recvCount == 0) {
        logError("Error receiving broadcast.", SupplierLogName(supplier));
        return;
    }
    // we won't do anything on udp with supplier
}

void newConnectionHandler(int fd, Supplier* supplier, FdSet* fdset) {
    logInfo("New connection request.", SupplierLogName(supplier));
    int newfd = accClient(fd);
    if (newfd < 0) {
        logError("Error accepting new connection.", SupplierLogName(supplier));
        return;
    }
    FD_SETTER(newfd, fdset);
    logInfo("New connection accepted.", SupplierLogName(supplier));
}

void yesNoPrompt(char* name, int quantity, unsigned short port, char* supplierName) {
    char msg[BUF_MSG] = {STRING_END};

    char ans[BUF_MSG];
    getInput(STDIN_FILENO, "Accept? (y/n)", ans, BUF_MSG);

    int ansFd = connectServer(port);

    // result-name:quantity
    if (!strcmp(ans, "y")) {
        logInfo("Accept request", supplierName);
        memset(msg, STRING_END, BUF_MSG);
        sprintf(msg, "%s-%s:%d", ACCEPTED_MSG, name, quantity);
        send(ansFd, msg, strlen(msg), 0);
    } else if (!strcmp(ans, "n")) {
        logInfo("Reject request", supplierName);
        memset(msg, STRING_END, BUF_MSG);
        sprintf(msg, "%s-%s:%d", REJECTED_MSG, name, quantity);
        send(ansFd, msg, strlen(msg), 0);
    } else {
        logError("Invalid answer.", supplierName);
    }
}

void chatHandler(int fd, Supplier* supplier, FdSet* fdset) {
    char msgBuf[BUF_MSG] = {STRING_END};
    int recvCount = recv(fd, msgBuf, BUF_MSG, 0);
    if (recvCount == 0) {
        logInfo("Connection closed.", SupplierLogName(supplier));
        close(fd);
        FD_CLRER(fd, fdset);
        return;
    }

    // port:ingredient:quantity
    int port = atoi(strtok(msgBuf, REQ_DELIM));
    char* name = strtok(NULL, REQ_DELIM);
    int quantity = atoi(strtok(NULL, REQ_DELIM));
    char msg[BUF_MSG] = {STRING_END};
    sprintf(msg, "You have new request for %s with quantity of %d from port %d", name, quantity, port);
    logMsg(msg, SupplierLogName(supplier));

    yesNoPrompt(name, quantity, port, SupplierLogName(supplier));
}

void interface(Supplier* supplier) {
    FdSet fdset;
    InitFdSet(&fdset, supplier->bcast.fd, supplier->tcpFd);
    
    while (1) {
        cliPrompt();
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
            else if (i == supplier->tcpFd)
                newConnectionHandler(i, supplier, &fdset);
            else
                chatHandler(i, supplier, &fdset);
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