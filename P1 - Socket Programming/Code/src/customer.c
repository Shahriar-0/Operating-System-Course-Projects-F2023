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
    logInfo("Initializing broadcast for customer.");
    int bcfd = initBroadcast(&customer->bcast.addr);
    if (bcfd < 0) return bcfd;
    customer->bcast.fd = bcfd;

    // getting into of all restaurants
    char* msg = REG_MSG;
    // TODO: check for uniqueness among customers
    logInfo("Broadcast for customer initialized.");
}

void loadFoodNames(Customer* customer) {
    logInfo("Loading food names.");
    cJSON* root = loadJSON();
    if (root == NULL) return;

    int foodSize = cJSON_GetArraySize(root);
    customer->foodSize = foodSize;

    cJSON* item = NULL;
    int index = 0;
    cJSON_ArrayForEach(item, root) {
        customer->foods[index] = strdup(item->string);
        index++;
    }

    cJSON_Delete(root);
    logInfo("Food names loaded.");
}

void printMenuSummary(const Customer* customer) {
    logInfo("Printing menu summary.");
    logLamination();
    logNormal("Menu:\n");
    char Food[BUF_MSG] = {'\0'};
    for (int i = 0; i < customer->foodSize; i++) {
        memset(Food, STRING_END, BUF_MSG);
        sprintf(Food, "%d. %s", i + 1, customer->foods[i]);
        logNormal(Food);
    }
    logLamination();
    logInfo("Menu summary printed.");
}

void initCustomer(Customer* customer, char* port) {
    logInfo("Initializing customer.");
    initBroadcastCustomer(customer);

    customer->tcpPort = atoi(port);
    initTCP(&customer->tcpPort);

    loadFoodNames(customer);

    logInfo("Customer initialized.");
}

int main(int argc, char** argv) {
    if (argc != 2) {
        logError("Usage: ./customer <port>");
        exit(EXIT_FAILURE);
    }

    Customer customer;
    initCustomer(&customer, argv[1]);

    // struct sigaction sigact = {.sa_handler = handleTimeout, .sa_flags = SA_RESTART};
    // sigaction(SIGALRM, &sigact, NULL);
}