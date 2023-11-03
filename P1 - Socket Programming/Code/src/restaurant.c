#include "define.h"
#include "network.h"
#include "utils.h"

int chatSocket = -1;
int timedOut = 0;

char* RestaurantLogName(Restaurant* restaurant) {
    char* name[BUF_NAME];
    EXT type = (restaurant->state == OPEN) ? RESTAURANT : RESTAURANT_CLOSE;
    sprintf(name, "%s%s%d%s%d", restaurant->name, NAME_DELIM, restaurant->tcpPort, NAME_DELIM,
            type);
    return strdup(name);
}

void endConnection(FdSet* fdset, Restaurant* restaurant) {
    logWarning("Timeout (90s): Disconnected from discussion.", RestaurantLogName(restaurant));
    alarm(0);
    FD_CLRER(chatSocket, fdset);
    close(chatSocket);
    chatSocket = -1;
}

void handleTimeout(int sig) {
    timedOut = 1;
    close(chatSocket);
}

void exiting(Restaurant* restaurant) { exitall(RestaurantLogName(restaurant)); }

loadMenu(Restaurant* restaurant) {
    logInfo("Loading menu.", RestaurantLogName(restaurant));
    cJSON* root = loadJSON();
    if (root == NULL) return;

    int menuSize = 0;
    cJSON* food_item = NULL;
    cJSON_ArrayForEach(food_item, root) {
        Food* food = &restaurant->menu[menuSize];
        strncpy(food->name, food_item->string, BUF_NAME);

        int ingredientSize = 0;
        cJSON* ingredient_item = NULL;
        cJSON_ArrayForEach(ingredient_item, food_item) {
            food->ingredients[ingredientSize] = strdup(ingredient_item->string);
            food->quantity[ingredientSize] = ingredient_item->valueint;
            ingredientSize++;
        }
        food->ingredientSize = ingredientSize;

        menuSize++;
    }
    restaurant->menuSize = menuSize;

    cJSON_Delete(root);
    logInfo("Menu loaded.", RestaurantLogName(restaurant));
}

void addIngredient(Restaurant* restaurant, char* ingredientName, int quantity) {
    logInfo("Adding ingredient.", RestaurantLogName(restaurant));
    for (int i = 0; i < restaurant->ingredientSize; i++) {
        if (!strcmp(restaurant->ingredients[i], ingredientName)) {
            restaurant->quantity[i] += quantity;
            logInfo("Ingredient added.", RestaurantLogName(restaurant));
            return;
        }
    }
    restaurant->ingredients[restaurant->ingredientSize] = strdup(ingredientName);
    restaurant->quantity[restaurant->ingredientSize] = quantity;
    restaurant->ingredientSize++;
    logInfo("Ingredient added.", RestaurantLogName(restaurant));
}

int canServeFood(Restaurant* restaurant, FoodRequest* foodRequest) {
    logInfo("Checking if restaurant can serve food.", RestaurantLogName(restaurant));
    for (int i = 0; i < restaurant->menuSize; i++) {
        Food* food = &restaurant->menu[i];
        if (!strcmp(food->name, foodRequest->foodName)) {
            for (int j = 0; j < food->ingredientSize; j++) {
                char* ingredientName = food->ingredients[j];
                int quantity = food->quantity[j];
                int haveIngredient = 0;
                for (int k = 0; k < restaurant->ingredientSize; k++) {
                    if (!strcmp(restaurant->ingredients[k], ingredientName)) {
                        if (restaurant->quantity[k] > quantity) 
                            haveIngredient = 1;
                    }
                }
                if (!haveIngredient) {
                    logError("Restaurant doesn't have enough ingredient.", RestaurantLogName(restaurant));
                    return 0;
                }
            }
            logInfo("Restaurant can serve food.", RestaurantLogName(restaurant));
            return 1;
        }
    }
    logError("Restaurant doesn't have food.", RestaurantLogName(restaurant));
    return 0;
}

void serveFood(Restaurant* restaurant, FoodRequest* foodRequest) {
    logInfo("Serving food.", RestaurantLogName(restaurant));
    for (int i = 0; i < restaurant->menuSize; i++) {
        Food* food = &restaurant->menu[i];
        if (!strcmp(food->name, foodRequest->foodName)) {
            for (int j = 0; j < food->ingredientSize; j++) {
                char* ingredientName = food->ingredients[j];
                int quantity = food->quantity[j];
                for (int k = 0; k < restaurant->ingredientSize; k++) {
                    if (!strcmp(restaurant->ingredients[k], ingredientName)) {
                        restaurant->quantity[k] -= quantity;
                    }
                }
            }
            return;
        }
    }
    logInfo("Food can't be served.", RestaurantLogName(restaurant));
}

void addPendingRequest(Restaurant* restaurant, FoodRequest* foodRequest) {
    logInfo("Adding pending request.", RestaurantLogName(restaurant));
    logInfo("Pending request added.", RestaurantLogName(restaurant));
}

void logFood(Restaurant* restaurant, FoodRequest* foodRequest, RequestState state) {
    logInfo("Logging food.", RestaurantLogName(restaurant));
    restaurant->handledRequests[restaurant->handledRequestsSize] = *foodRequest;
    restaurant->handledRequests[restaurant->handledRequestsSize].state = state;
    restaurant->handledRequestsSize++;
    logInfo("Food logged.", RestaurantLogName(restaurant));
}

void printMenu(const Restaurant* restaurant) {
    logInfo("Printing menu.", RestaurantLogName(restaurant));
    logLamination();
    logNormal("Menu:\n");
    for (int i = 0; i < restaurant->menuSize; i++) {
        const Food* food = &restaurant->menu[i];
        char buf[BUF_MSG] = {STRING_END};
        sprintf(buf, "%d. %s:", i + 1, food->name);
        logNormal(buf);
        for (int j = 0; j < food->ingredientSize; j++) {
            memset(buf, STRING_END, BUF_MSG);
            sprintf(buf, "     - %s: %d", food->ingredients[j], food->quantity[j]);
            logNormal(buf);
        }
    }
    logLamination();
    logInfo("Menu printed.", RestaurantLogName(restaurant));
}

void broadcast(Restaurant* restaurant, char* msg) {
    sendto(restaurant->bcast.fd, msg, strlen(msg), 0, (struct sockaddr*)&restaurant->bcast.addr,
           sizeof(restaurant->bcast.addr));
}

int initBroadcastRestaurant(Restaurant* restaurant) {
    logInfo("Initializing broadcast for restaurant.", RestaurantLogName(restaurant));
    int bcfd = initBroadcast(&restaurant->bcast.addr);
    if (bcfd < 0) return bcfd;
    restaurant->bcast.fd = bcfd;

    logInfo("Broadcast for restaurant initialized.", RestaurantLogName(restaurant));
}

void initRestaurant(Restaurant* restaurant, char* port) {
    getInput(STDIN_FILENO, "Enter restaurant name: ", restaurant->name, BUF_NAME);
    if (!isUniqueName(restaurant->name)) {
        write(1, "Name already taken.\n", strlen("Name already taken.\n"));
        exit(EXIT_FAILURE);
    }
    restaurant->tcpPort = atoi(port);

    logInfo("Initializing restaurant.", RestaurantLogName(restaurant));

    initBroadcastRestaurant(restaurant);

    restaurant->tcpFd = initTCP(restaurant->tcpPort);

    restaurant->handledRequestsSize = restaurant->ingredientSize = restaurant->menuSize = 0;

    loadMenu(restaurant);
    restaurant->state = OPEN;
    logInfo("Restaurant initialized.", RestaurantLogName(restaurant));
}

void printHelp(Restaurant* restaurant) {
    logLamination();
    logNormal("Available commands:");
    logNormal("    menu: print menu");
    logNormal("    open: open restaurant");
    logNormal("    close: close restaurant");
    logNormal("    ingredients: print ingredients");
    logNormal("    handled: print handled requests");
    logNormal("    order: order ingredient");
    logNormal("    suppliers: print suppliers");
    logNormal("    exit: exit program");
    logLamination();
}

void openRestaurant(Restaurant* restaurant) {
    if (restaurant->state == OPEN) {
        logError("Restaurant is already open.", RestaurantLogName(restaurant));
        return;
    }
    char* src = RestaurantLogName(restaurant);
    restaurant->state = OPEN;
    char* dst = RestaurantLogName(restaurant);
    char srcaddr[BUF_NAME], dstaddr[BUF_NAME];
    sprintf(srcaddr, "%s%s%s", LOG_FOLDER_ADD, src, LOG_EXT);
    sprintf(dstaddr, "%s%s%s", LOG_OVER_ADD, dst, LOG_EXT);

    rename(src, dst);
    logInfo("Restaurant opened.", RestaurantLogName(restaurant));

    char msg[BUF_MSG] = {STRING_END};
    sprintf(msg, "Restaurant %s opened on port %d.", restaurant->name, restaurant->tcpPort);
    broadcast(restaurant, msg);
}

void closeRestaurant(Restaurant* restaurant) {
    if (restaurant->state == CLOSED) {
        logError("Restaurant is already closed.", RestaurantLogName(restaurant));
        return;
    }
    char* src = RestaurantLogName(restaurant);
    restaurant->state = CLOSED;
    char* dst = RestaurantLogName(restaurant);
    char srcaddr[BUF_NAME], dstaddr[BUF_NAME];
    sprintf(srcaddr, "%s%s%s", LOG_FOLDER_ADD, src, LOG_EXT);
    sprintf(dstaddr, "%s%s%s", LOG_OVER_ADD, dst, LOG_EXT);

    rename(srcaddr, dstaddr);
    logInfo("Restaurant closed.", RestaurantLogName(restaurant));

    char msg[BUF_MSG] = {STRING_END};
    sprintf(msg, "Restaurant %s closed on port %d.", restaurant->name, restaurant->tcpPort);
    broadcast(restaurant, msg);
}

void printIngredients(const Restaurant* restaurant) {
    logInfo("Printing ingredients.", RestaurantLogName(restaurant));
    logLamination();
    logNormal("Ingredients:");
    for (int i = 0; i < restaurant->ingredientSize; i++) {
        char buf[BUF_MSG] = {STRING_END};
        sprintf(buf, "%d. %s: %d", i + 1, restaurant->ingredients[i], restaurant->quantity[i]);
        logNormal(buf);
    }
    logLamination();
    logInfo("Ingredients printed.", RestaurantLogName(restaurant));
}

void printHandledRequests(Restaurant* restaurant) {
    logLamination();
    logNormal("Food requests:");
    for (int i = 0; i < restaurant->handledRequestsSize; i++) {
        char buf[BUF_MSG] = {STRING_END};
        sprintf(buf, "%d. Customer : %s - food : %s -  state : %s", i + 1, restaurant->handledRequests[i].customerName,
                restaurant->handledRequests[i].foodName,
                (restaurant->handledRequests[i].state == ACCEPTED ? "ACCEPTED" : "REJECTED"));
        logNormal(buf);
    }
    logLamination();
}

void printSuppliers(Restaurant* restaurant) { printWithType(SUPPLIER); }

void orderIngredient(Restaurant* restaurant) {
    char token[BUF_NAME];
    char ingredientName[BUF_NAME] = {STRING_END};
    int quantity = 0;
    int port;
    getInput(STDIN_FILENO, "Enter ingredient name: ", ingredientName, BUF_NAME);
    getInput(STDIN_FILENO, "Enter quantity: ", token, BUF_MSG);
    quantity = atoi(token);
    getInput(STDIN_FILENO, "Enter supplier port: ", token, BUF_MSG);
    port = atoi(token);
    int serverFd = connectServer(port);

    // port:ingredient:quantity
    char msg[BUF_MSG] = {STRING_END};
    sprintf(msg, "%d%s%s%s%d", restaurant->tcpPort, REQ_DELIM, ingredientName, REQ_DELIM, quantity);
    alarm(TIME_OUT);
    send(serverFd, msg, strlen(msg), 0);
    logInfo("Ingredient request sent.", RestaurantLogName(restaurant));
}

void cli(Restaurant* restaurant, FdSet* fdset) {
    char msgBuf[BUF_MSG] = {STRING_END};
    getInput(STDIN_FILENO, NULL, msgBuf, BUF_MSG);

    if (!strcmp(msgBuf, "help"))
        printHelp(restaurant);
    else if (!strcmp(msgBuf, "menu"))
        printMenu(restaurant);
    else if (!strcmp(msgBuf, "open"))
        openRestaurant(restaurant);
    else if (!strcmp(msgBuf, "close"))
        closeRestaurant(restaurant);
    else if (!strcmp(msgBuf, "ingredients"))
        printIngredients(restaurant);
    else if (!strcmp(msgBuf, "handled"))
        printHandledRequests(restaurant);
    else if (!strcmp(msgBuf, "suppliers"))
        printSuppliers(restaurant);
    else if (!strcmp(msgBuf, "order"))
        orderIngredient(restaurant);
    else if (!strcmp(msgBuf, "exit"))
        exiting(restaurant);
    else
        logError("Invalid command.", RestaurantLogName(restaurant));
}

void UDPHandler(Restaurant* restaurant, FdSet* fdset) {
    char msgBuf[BUF_MSG] = {STRING_END};
    int recvCount = recvfrom(restaurant->bcast.fd, msgBuf, BUF_MSG, 0, NULL, NULL);
    if (recvCount == 0) {
        logError("Error receiving broadcast.", RestaurantLogName(restaurant));
        return;
    }

    // we won't do anything on udp with restaurant
}

void newConnectionHandler(int fd, Restaurant* restaurant, FdSet* fdset) {
    logInfo("New connection request.", RestaurantLogName(restaurant));
    int newfd = accClient(fd);
    if (newfd < 0) {
        logError("Error accepting new connection.", RestaurantLogName(restaurant));
        return;
    }
    FD_SETTER(newfd, fdset);
    logInfo("New connection accepted.", RestaurantLogName(restaurant));
}

void yesNoPrompt(unsigned short port, Restaurant* restaurant, FoodRequest* foodRequest) {
    char msg[BUF_MSG] = {STRING_END};

    char ans[BUF_MSG];
    getInput(STDIN_FILENO, "Accept? (y/n)", ans, BUF_MSG);

    int ansFd = connectServer(port);

    if (!strcmp(ans, "y")) {
        if (restaurant->state == OPEN && canServeFood(restaurant, foodRequest)) {
        logInfo("Accept request", RestaurantLogName(restaurant));
            sprintf(msg, "%s", ACCEPTED_MSG);
            serveFood(restaurant, foodRequest);
            send(ansFd, msg, strlen(msg), 0);
            logFood(restaurant, foodRequest, ACCEPTED);
        } else {
            logInfo("Reject request", RestaurantLogName(restaurant));
            sprintf(msg, "%s", REJECTED_MSG);
            send(ansFd, msg, strlen(msg), 0);
            logFood(restaurant, foodRequest, REJECTED);
        }
    } else if (!strcmp(ans, "n")) {
        logInfo("Reject request", RestaurantLogName(restaurant));
        sprintf(msg, "%s", REJECTED_MSG);
        send(ansFd, msg, strlen(msg), 0);
        logFood(restaurant, foodRequest, REJECTED);
    } else {
        logError("Invalid answer.", RestaurantLogName(restaurant));
    }
}

void chatHandler(int fd, Restaurant* restaurant, FdSet* fdset) {
    char msgBuf[BUF_MSG] = {STRING_END};
    int recvCount = recv(fd, msgBuf, BUF_MSG, 0);
    if (recvCount == 0) {
        logInfo("Connection closed.", RestaurantLogName(restaurant));
        close(fd);
        FD_CLRER(fd, fdset);
        return;
    }
    char* type = strtok(msgBuf, REQ_IN_DELIM);

    if (!strcmp(type, ACCEPTED_MSG)) {
        alarm(0);
        logMsg("Ingredient request accepted.", RestaurantLogName(restaurant));
        char* name = strtok(NULL, REQ_DELIM);
        int quantity = atoi(strtok(NULL, REQ_DELIM));
        addIngredient(restaurant, name, quantity);
    } else if (!strcmp(type, REJECTED_MSG)) {
        alarm(0);
        logMsg("Ingredient request rejected.", RestaurantLogName(restaurant));
    } else if (!strcmp(type, REQUEST_MSG)) {
        logMsg("Food order request received.", RestaurantLogName(restaurant));
        int customerPort = atoi(strtok(NULL, REQ_DELIM));
        char* customerName = strtok(NULL, REQ_DELIM);
        char* foodName = strtok(NULL, REQ_DELIM);
        char msg[BUF_MSG] = {STRING_END};
        sprintf(msg, "You have new request for %s from %s from port %d", foodName, customerName,
                customerPort);
        logMsg(msg, RestaurantLogName(restaurant));
        FoodRequest foodRequest = {STRING_END};
        strncpy(foodRequest.customerName, customerName, BUF_NAME);
        strncpy(foodRequest.foodName, foodName, BUF_NAME);
        foodRequest.customerPort = customerPort;
        yesNoPrompt(customerPort, restaurant, &foodRequest);
    } else {
        logError("Invalid message received.", RestaurantLogName(restaurant));
    }
}

void interface(Restaurant* restaurant) {
    FdSet fdset;
    InitFdSet(&fdset, restaurant->bcast.fd, restaurant->tcpFd);

    while (1) {
        cliPrompt();
        fdset.working = fdset.master;
        select(fdset.max + 1, &fdset.working, NULL, NULL, NULL);

        if (timedOut) {
            timedOut = 0;
            endConnection(&fdset, restaurant);
        }

        for (int i = 0; i <= fdset.max; ++i) {
            if (!FD_ISSET(i, &fdset.working)) continue;

            // this if is for having a clean interface when receiving messages
            if (i != STDIN_FILENO) write(STDOUT_FILENO, CLEAR_LINE_ANSI, CLEAR_LINE_LEN);

            if (i == STDIN_FILENO)
                cli(restaurant, &fdset);
            else if (i == restaurant->bcast.fd)
                UDPHandler(restaurant, &fdset);
            else if (i == restaurant->tcpFd)
                newConnectionHandler(i, restaurant, &fdset);
            else
                chatHandler(i, restaurant, &fdset);
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        perror("Usage: ./supplier <port>");
        exit(EXIT_FAILURE);
    }

    Restaurant restaurant;
    restaurant.tcpPort = atoi(argv[1]);
    initRestaurant(&restaurant, argv[1]);

    struct sigaction sigact = {.sa_handler = handleTimeout, .sa_flags = SA_RESTART};
    sigaction(SIGALRM, &sigact, NULL);

    interface(&restaurant);
}
