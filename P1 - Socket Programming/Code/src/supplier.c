#include "define.h"
#include "network.h"
#include "utils.h"

int initBroadcastSupplier(Supplier* supplier) {
    logInfo("Initializing broadcast for supplier.");
    int bcfd = initBroadcast(&supplier->bcast.addr);
    if (bcfd < 0) return bcfd;
    supplier->bcast.fd = bcfd;

    // getting into of all restaurants
    char* msg = REG_MSG;
    // TODO: check for uniqueness among suppliers
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

void interface(Supplier* supplier) {
    char msgBuf[BUF_MSG] = {STRING_END};
    while (1) {
        cliPrompt();
        memset(msgBuf, STRING_END, BUF_MSG);
        fdset.working = fdset.master;
        select(fdset.max + 1, &fdset.working, NULL, NULL, NULL);

        for (int i = 0; i <= fdset.max; ++i) {
            if (!FD_ISSET(i, &fdset.working)) continue;
            if (i != STDIN_FILENO) {
                write(STDOUT_FILENO, "\x1B[2K\r", 5);
            }
            if (i == STDIN_FILENO) {
                cli(supplier, &fdset, &pendingServers);
            }
            else if (FD_ISSET(i, &pendingServers)) {
                int accSocket = accClient(i);
                close(i);

                FD_CLR(i, &pendingServers);
                FD_CLRER(i, &fdset);
                FD_SETTER(accSocket, &fdset);

                int prodNum = getProductBySocket(&supplier->prods, i);
                supplier->prods.ptr[prodNum].state = DISCUSS;
                supplier->prods.ptr[prodNum].socket = accSocket;
                sendMsgBcast(supplier, serializeBcast(supplier, prodNum));

                snprintf(msgBuf, BUF_MSG, "Accepted client for product #%d: %s", prodNum + 1, supplier->prods.ptr[prodNum].name);
                logInfo(msgBuf);
            }
            else {
                int crecv = recv(i, msgBuf, BUF_MSG, 0);

                if (crecv == 0) {
                    endConnection(i, supplier, &fdset, &pendingServers);
                    logInfo("Discussion closed by client.");
                    continue;
                }

                handleChat(msgBuf, i, supplier);
            }
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