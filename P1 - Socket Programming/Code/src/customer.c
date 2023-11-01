#include "define.h"
#include "network.h"
#include "utils.h"


int timedOut = 0;
int chatSocket = -1;

void handleTimeout(int sig) {
    logWarning("Timeout (90s): Disconnected from discussion.");
    timedOut = 1;
    close(chatSocket);
}


int initBroadcastCustomer(Customer* customer) {
    int bcfd = initBroadcast(&customer->bcast.addr);
    if (bcfd < 0) return bcfd;
    customer->bcast.fd = bcfd;
    
    // getting into of all restaurants
    char* msg = REG_MSG;
    // TODO: check for uniqueness among customers
}

void printMenuSummary(const Customer* customer) {
    logLamination();
    logNormal("Menu:\n");
    char Food[BUF_MSG] = {'\0'};
    for (int i = 0; i < customer->foodSize; i++) {
        memset(Food, STRING_END, BUF_MSG);
        sprintf(Food, "%d. %s\n", i + 1, customer->foods[i]);
        logNormal(Food);
    }
    logLamination();
}

void initCustomer(Customer* customer, char* port) {
    initBroadcastCustomer(customer);

    customer->tcpPort = atoi(port);
    initTCP(&customer->tcpPort);

    loadFoodNames(customer);
    printMenuSummary(customer); // * for testing
}

int main(int argc, char** argv) {
    if (argc != 2) {
        logError("Usage: ./customer <port>");
        exit(EXIT_FAILURE);
    }

    Customer customer;
    initCustomer(&customer, argv[1]);


    struct sigaction sigact = {.sa_handler = handleTimeout, .sa_flags = SA_RESTART};
    sigaction(SIGALRM, &sigact, NULL);
    

    // int ret = initBroadcastCustomer(&customer);
    // if (ret < 0) {
    //     logError("Failed to initialize broadcast.");
    //     exit(EXIT_FAILURE);
    // }
}