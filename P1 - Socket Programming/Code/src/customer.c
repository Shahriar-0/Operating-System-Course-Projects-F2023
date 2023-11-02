#include "define.h"
#include "network.h"
#include "utils.h"

int timedOut = 0;
int chatSocket = -1;
char* NAME;

char* CustomerLogName(Customer* customer) {
    char* name[BUF_NAME];
    sprintf(name, "%s%s%d%s%d", customer->name, NAME_DELIM, customer->tcpPort, NAME_DELIM,
            CUSTOMER);
    return strdup(name);
}

void exiting(Customer* customer) { exitall(CustomerLogName(customer)); }

void handleTimeout(int sig) {
    perror("Timeout (90s): Disconnected from discussion.");
    timedOut = 1;
    close(chatSocket);
}

void broadcast(Customer* customer, char* msg) {
    sendto(customer->bcast.fd, msg, strlen(msg), 0, (struct sockaddr*)&customer->bcast.addr,
           sizeof(customer->bcast.addr));
}

int initBroadcastCustomer(Customer* customer) {
    logInfo("Initializing broadcast for customer.", CustomerLogName(customer));
    int bcfd = initBroadcast(&customer->bcast.addr);
    if (bcfd < 0) return bcfd;
    customer->bcast.fd = bcfd;

    logInfo("Broadcast for customer initialized.", CustomerLogName(customer));
}

void loadFoodNames(Customer* customer) {
    logInfo("Loading food names.", CustomerLogName(customer));
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
    logInfo("Food names loaded.", CustomerLogName(customer));
}

void printMenuSummary(const Customer* customer) {
    logInfo("Printing menu summary.", CustomerLogName(customer));
    logLamination();
    logNormal("Menu:\n", CustomerLogName(customer));
    char Food[BUF_MSG] = {'\0'};
    for (int i = 0; i < customer->foodSize; i++) {
        memset(Food, STRING_END, BUF_MSG);
        sprintf(Food, "%d. %s", i + 1, customer->foods[i]);
        logNormal(Food, CustomerLogName(customer));
    }
    logLamination();
    logInfo("Menu summary printed.", CustomerLogName(customer));
}

void initCustomer(Customer* customer, char* port) {
    getInput(STDIN_FILENO, "Enter your name: ", customer->name, BUF_NAME);
    if (!isUniqueName(customer->name)) {
        write(1, "Name already taken.\n", strlen("Name already taken.\n"));
        exit(EXIT_FAILURE);
    }
    customer->tcpPort = atoi(port);

    logInfo("Initializing customer.", CustomerLogName(customer));
    initBroadcastCustomer(customer);
    
    customer->tcpFd = initTCP(customer->tcpPort);

    loadFoodNames(customer);

    customer->restaurantSize = 0;

    logInfo("Customer initialized.", CustomerLogName(customer));
}

char* generateFoodRequest(Customer* customer, char* foodName) {
    char msg[BUF_MSG] = {STRING_END};

    // clang-format off
    // foodName|customerPort
    snprintf(msg, BUF_MSG, "%s%c%d", 
             foodName, BCAST_IN_DELIM, 
             customer->tcpPort);
    // clang-format on

    return strdup(msg);
}

void orderFood(Customer* customer, char* msg) {
    char food[BUF_NAME] = {STRING_END};
    getInput(STDIN_FILENO, "Enter food name: ", food, BUF_NAME);
    char portStr[BUF_CLI] = {STRING_END};
    getInput(STDIN_FILENO, "Enter restaurant port: ", portStr, BUF_CLI);
    unsigned short port = atoi(portStr);

    int fd = connectServer(port);
    char* req = generateFoodRequest(customer, food);
    send(fd, req, strlen(req), 0);

    logInfo("Order sent.", CustomerLogName(customer));
}

void printHelp(Customer* customer) {
    logLamination();
    logNormal("Commands:\n", CustomerLogName(customer));
    logNormal("    help: print this help.\n", CustomerLogName(customer));
    logNormal("    menu: print the menu summary.\n", CustomerLogName(customer));
    logNormal("    order: order food.\n", CustomerLogName(customer));
    logNormal("    restaurants: print the list of restaurants.\n", CustomerLogName(customer));
    logNormal("    exit: exit the program.\n", CustomerLogName(customer));
    logLamination();
}

void printRestaurants(Customer* customer) { printWithType(RESTAURANT); }

void cli(Customer* customer, FdSet* fdset) {
    char msg[BUF_MSG] = {STRING_END};

    getInput(STDIN_FILENO, NULL, msg, BUF_MSG);

    if (!strcmp(msg, "help"))
        printHelp(customer);
    else if (!strcmp(msg, "menu"))
        printMenuSummary(customer);
    else if (!strcmp(msg, "order"))
        orderFood(customer, msg);
    else if (!strcmp(msg, "restaurants"))
        printRestaurants(customer);
    else if (!strcmp(msg, "exit"))
        exiting(customer);
    else
        logError("Invalid command.", CustomerLogName(customer));
}

void addRestaurant(Customer* customer, char* name, int port) {
    strcpy(customer->restaurants[customer->restaurantSize].name, name);
    customer->restaurants[customer->restaurantSize].port = port;
    customer->restaurantSize++;
}

void removeRestaurant(Customer* customer, char* name) {
    for (int i = 0; i < customer->restaurantSize; i++) {
        if (!strcmp(customer->restaurants[i].name, name)) {
            for (int j = i; j < customer->restaurantSize - 1; j++) {
                customer->restaurants[j] = customer->restaurants[j + 1];
            }
            customer->restaurantSize--;
            break;
        }
    }
}

void UDPHandler(Customer* customer, FdSet* fdset) {
    char msgBuf[BUF_MSG] = {STRING_END};
    int recvCount = recvfrom(customer->bcast.fd, msgBuf, BUF_MSG, 0, NULL, NULL);
    if (recvCount == 0) {
        logError("Error receiving broadcast.", CustomerLogName(customer));
        return;
    }

    logLamination();
    logNormal("Broadcast message:\n", CustomerLogName(customer));
    logNormal(msgBuf, CustomerLogName(customer));
    logLamination();
}

void newConnectionHandler(int fd, Customer* customer, FdSet* fdset) {
    logInfo("New connection request.", CustomerLogName(customer));
    int newfd = accClient(fd);
    if (newfd < 0) {
        logError("Error accepting new connection.", CustomerLogName(customer));
        return;
    }
    FD_SETTER(newfd, fdset);
    logInfo("New connection accepted.", CustomerLogName(customer));
}

void chatHandler(int fd, Customer* customer, FdSet* fdset) {
    char msgBuf[BUF_MSG] = {STRING_END};
    int recvCount = recv(fd, msgBuf, BUF_MSG, 0);
    if (recvCount == 0) {
        logInfo("Connection closed.", CustomerLogName(customer));
        close(fd);
        FD_CLRER(fd, fdset);
        return;
    }

}

void interface(Customer* customer) {
    FdSet fdset;
    InitFdSet(&fdset, customer->bcast.fd, customer->tcpPort);

    while (1) {
        cliPrompt();
        fdset.working = fdset.master;
        select(fdset.max + 1, &fdset.working, NULL, NULL, NULL);

        for (int i = 0; i <= fdset.max; ++i) {
            if (!FD_ISSET(i, &fdset.working)) continue;

            // this if is for having a clean interface when receiving messages
            if (i != STDIN_FILENO) write(STDOUT_FILENO, CLEAR_LINE_ANSI, CLEAR_LINE_LEN);

            if (i == STDIN_FILENO)
                cli(customer, &fdset);
            else if (i == customer->bcast.fd)
                UDPHandler(customer, &fdset);
            else if (i == customer->tcpFd)
                newConnectionHandler(i, customer, &fdset);
            else
                chatHandler(i, customer, &fdset);
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        perror("Usage: ./customer <port>");
        exit(EXIT_FAILURE);
    }

    Customer customer;
    initCustomer(&customer, argv[1]);

    struct sigaction sigact = {.sa_handler = handleTimeout, .sa_flags = SA_RESTART};
    sigaction(SIGALRM, &sigact, NULL);

    interface(&customer);
}