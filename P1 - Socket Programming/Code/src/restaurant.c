#include "define.h"
#include "network.h"
#include "utils.h"

char* RestaurantLogName(Restaurant* restaurant) {
    char* name[BUF_NAME];
    EXT type = (restaurant->state == OPEN) ? RESTAURANT : RESTAURANT_CLOSE;
    sprintf(name, "%s%s%d%s%d", restaurant->name, NAME_DELIM, restaurant->tcpPort, NAME_DELIM,
            type);
    return strdup(name);
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
    strncpy(restaurant->ingredients[restaurant->ingredientSize], ingredientName, BUF_NAME);
    restaurant->quantity[restaurant->ingredientSize] = quantity;
    restaurant->ingredientSize++;
    logInfo("Ingredient added.", RestaurantLogName(restaurant));
}

void canServeFood(Restaurant* restaurant, FoodRequest* foodRequest) {
    logInfo("Checking if restaurant can serve food.", RestaurantLogName(restaurant));
    for (int i = 0; i < restaurant->menuSize; i++) {
        Food* food = &restaurant->menu[i];
        if (!strcmp(food->name, foodRequest->foodName)) {
            for (int j = 0; j < food->ingredientSize; j++) {
                char* ingredientName = food->ingredients[j];
                int quantity = food->quantity[j];
                for (int k = 0; k < restaurant->ingredientSize; k++) {
                    if (!strcmp(restaurant->ingredients[k], ingredientName)) {
                        if (restaurant->quantity[k] < quantity) {
                            logInfo("Restaurant can't serve food.", RestaurantLogName(restaurant));
                            return;
                        }
                    }
                }
            }
            logInfo("Restaurant can serve food.", RestaurantLogName(restaurant));
            return;
        }
    }
    logInfo("Restaurant can't serve food.", RestaurantLogName(restaurant));
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
    restaurant->pendingRequests[restaurant->pendingRequestSize] = *foodRequest;
    restaurant->pendingRequestSize++;
    logInfo("Pending request added.", RestaurantLogName(restaurant));
}

void logFood(Restaurant* restaurant, FoodRequest* foodRequest, RequestState state) {
    logInfo("Food logged.", RestaurantLogName(restaurant));
    for (int i = 0; i < restaurant->pendingRequestSize; i++) {
        FoodRequest* pendingRequest = &restaurant->pendingRequests[i];
        if (!strcmp(pendingRequest->customerName, foodRequest->customerName) &&
            !strcmp(pendingRequest->foodName, foodRequest->foodName)) {
            for (int j = i; j < restaurant->pendingRequestSize - 1; j++) {
                restaurant->pendingRequests[j] = restaurant->pendingRequests[j + 1];
            }
            restaurant->pendingRequestSize--;
            restaurant->handledRequests[restaurant->handledRequestsSize] = *pendingRequest;
            restaurant->handledRequests[restaurant->handledRequestsSize].state = state;
            restaurant->handledRequestsSize++;
            return;
        }
    }
    logError("Food can't be logged.", RestaurantLogName(restaurant));
}


void printMenu(const Restaurant* restaurant) {
    logInfo("Printing menu.", RestaurantLogName(restaurant));
    logLamination();
    logNormal("Menu:\n", RestaurantLogName(restaurant));
    for (int i = 0; i < restaurant->menuSize; i++) {
        const Food* food = &restaurant->menu[i];
        char buf[BUF_MSG] = {STRING_END};
        sprintf(buf, "%d. %s:", i + 1, food->name);
        logNormal(buf, RestaurantLogName(restaurant));
        for (int j = 0; j < food->ingredientSize; j++) {
            memset(buf, STRING_END, BUF_MSG);
            sprintf(buf, "     - %s: %d", food->ingredients[j], food->quantity[j]);
            logNormal(buf, RestaurantLogName(restaurant));
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

    initTCP(&restaurant->tcpPort);

    loadMenu(restaurant);
    restaurant->state = OPEN;
    logInfo("Restaurant initialized.", RestaurantLogName(restaurant));
}

void printHelp(Restaurant* restaurant) {
    logLamination();
    logNormal("Available commands:", RestaurantLogName(restaurant));
    logNormal("    menu: print menu", RestaurantLogName(restaurant));
    logNormal("    open: open restaurant", RestaurantLogName(restaurant));
    logNormal("    close: close restaurant", RestaurantLogName(restaurant));
    logNormal("    ingredients: print ingredients", RestaurantLogName(restaurant));
    logNormal("    pending: print pending requests", RestaurantLogName(restaurant));
    logNormal("    handled: print handled requests", RestaurantLogName(restaurant));
    logNormal("    suppliers: print suppliers", RestaurantLogName(restaurant));
    logNormal("    exit: exit program", RestaurantLogName(restaurant));
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
    logNormal("Ingredients:", RestaurantLogName(restaurant));
    for (int i = 0; i < restaurant->menuSize; i++) {
        const Food* food = &restaurant->menu[i];
        for (int j = 0; j < food->ingredientSize; j++) {
            char buf[BUF_MSG] = {STRING_END};
            sprintf(buf, "     - %s: %d", food->ingredients[j], food->quantity[j]);
            logNormal(buf, RestaurantLogName(restaurant));
        }
    }
    logLamination();
    logInfo("Ingredients printed.", RestaurantLogName(restaurant));
}

void printFoodRequests(Restaurant* restaurant, FoodRequest foodRequests[], int foodRequestSize) {
    logLamination();
    logNormal("Food requests:", RestaurantLogName(restaurant));
    for (int i = 0; i < foodRequestSize; i++) {
        char buf[BUF_MSG] = {STRING_END};
        sprintf(buf, "%d. Customer %s : food %s on port %d", i + 1, foodRequests[i].customerName,
                foodRequests[i].foodName, foodRequests[i].customerPort);
        logNormal(buf, RestaurantLogName(restaurant));
    }
    logLamination();
}

void printPendingRequests(Restaurant* restaurant) {
    logInfo("Printing pending requests.", RestaurantLogName(restaurant));
    printFoodRequests(restaurant, restaurant->pendingRequests, restaurant->pendingRequestSize);
    logInfo("Pending requests printed.", RestaurantLogName(restaurant));
}

void printHandledRequests(Restaurant* restaurant) {
    logInfo("Printing handled requests.", RestaurantLogName(restaurant));
    printFoodRequests(restaurant, restaurant->handledRequests, restaurant->handledRequestsSize);
    logInfo("Handled requests printed.", RestaurantLogName(restaurant));
}

void printSuppliers(Restaurant* restaurant) { printWithType(SUPPLIER); }

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
    else if (!strcmp(msgBuf, "pending"))
        printPendingRequests(restaurant);
    else if (!strcmp(msgBuf, "handled"))
        printHandledRequests(restaurant);
    else if (!strcmp(msgBuf, "suppliers"))
        printSuppliers(restaurant);
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
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int newfd = accept(fd, (struct sockaddr*)&addr, &addrlen);
    if (newfd < 0) {
        logError("Error accepting new connection.", RestaurantLogName(restaurant));
        return;
    }
    FD_SETTER(newfd, fdset);
    logInfo("New connection accepted.", RestaurantLogName(restaurant));
}

void chatHandler(int fd, char* msgBuf, Restaurant* restaurant, FdSet* fdset) {
    int recvCount = recv(fd, msgBuf, BUF_MSG, 0);
    if (recvCount == 0) {
        logInfo("Connection closed.", RestaurantLogName(restaurant));
        close(fd);
        FD_CLRER(fd, fdset);
        return;
    }
}

void interface(Restaurant* restaurant) {
    char msgBuf[BUF_MSG];

    FdSet fdset;
    InitFdSet(&fdset, restaurant->tcpPort);

    while (1) {
        cliPrompt();
        memset(msgBuf, '\0', BUF_MSG);
        fdset.working = fdset.master;
        select(fdset.max + 1, &fdset.working, NULL, NULL, NULL);

        for (int i = 0; i <= fdset.max; ++i) {
            if (!FD_ISSET(i, &fdset.working)) continue;

            // this if is for having a clean interface when receiving messages
            if (i != STDIN_FILENO) write(STDOUT_FILENO, CLEAR_LINE_ANSI, CLEAR_LINE_LEN);

            if (i == STDIN_FILENO)
                cli(restaurant, &fdset);
            else if (i == restaurant->bcast.fd)
                UDPHandler(restaurant, &fdset);
            else if (i == restaurant->tcpPort)
                newConnectionHandler(i, restaurant, &fdset);
            else
                chatHandler(i, msgBuf, restaurant, &fdset);
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

    interface(&restaurant);
}
