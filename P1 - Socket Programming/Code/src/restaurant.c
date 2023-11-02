#include "define.h"
#include "network.h"
#include "utils.h"

loadMenu(Restaurant* restaurant) {
    logInfo("Loading menu.", restaurant->name);
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
    logInfo("Menu loaded.", restaurant->name);
}

void printMenu(const Restaurant* restaurant) {
    logInfo("Printing menu.", restaurant->name);
    logLamination();
    logNormal("Menu:\n", restaurant->name);
    for (int i = 0; i < restaurant->menuSize; i++) {
        const Food* food = &restaurant->menu[i];
        char buf[BUF_MSG] = {STRING_END};
        sprintf(buf, "%d. %s:", i + 1, food->name);
        logNormal(buf, restaurant->name);
        for (int j = 0; j < food->ingredientSize; j++) {
            memset(buf, STRING_END, BUF_MSG);
            sprintf(buf, "     - %s: %d", food->ingredients[j], food->quantity[j]);
            logNormal(buf, restaurant->name);
        }
    }
    logLamination();
    logInfo("Menu printed.", restaurant->name);
}

void broadcast(Restaurant* restaurant, char* msg) {
    sendto(restaurant->bcast.fd, msg, strlen(msg), 0, (struct sockaddr*)&restaurant->bcast.addr,
           sizeof(restaurant->bcast.addr));
}

int initBroadcastRestaurant(Restaurant* restaurant) {
    logInfo("Initializing broadcast for restaurant.", restaurant->name);
    int bcfd = initBroadcast(&restaurant->bcast.addr);
    if (bcfd < 0) return bcfd;
    restaurant->bcast.fd = bcfd;

    broadcast(restaurant, REG_REQ_MSG);
    logInfo("Broadcast for restaurant initialized.", restaurant->name);
}

void initRestaurant(Restaurant* restaurant, char* port) {
    getInput(STDIN_FILENO, "Enter restaurant name: ", restaurant->name, BUF_NAME);
    
    logInfo("Initializing restaurant.", restaurant->name);
    
    initBroadcastRestaurant(restaurant);

    restaurant->tcpPort = atoi(port);
    initTCP(&restaurant->tcpPort);


    loadMenu(restaurant);
    restaurant->state = OPEN;
    logInfo("Restaurant initialized.", restaurant->name);
}

void printHelp(Restaurant* restaurant) {
    logNormal("Available commands:", restaurant->name);
    logNormal("    menu: print menu", restaurant->name);
    logNormal("    open: open restaurant", restaurant->name);
    logNormal("    close: close restaurant", restaurant->name);
    logNormal("    exit: exit program", restaurant->name);
}

void openRestaurant(Restaurant* restaurant) {
    if (restaurant->state == OPEN) {
        logError("Restaurant is already open.", restaurant->name);
        return;
    }
    restaurant->state = OPEN;
    logInfo("Restaurant opened.", restaurant->name);
}

void closeRestaurant(Restaurant* restaurant) {
    if (restaurant->state == CLOSED) {
        logError("Restaurant is already closed.", restaurant->name);
        return;
    }
    restaurant->state = CLOSED;
    logInfo("Restaurant closed.", restaurant->name);
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
    else if (!strcmp(msgBuf, "exit")) {
        logInfo("Exiting program.", restaurant->name);
        exit(EXIT_SUCCESS);
    } else 
        logError("Invalid command.", restaurant->name);
}

void UDPHandler(Restaurant* restaurant, FdSet* fdset) {
    char msgBuf[BUF_MSG] = {STRING_END};
    int recvCount = recvfrom(restaurant->bcast.fd, msgBuf, BUF_MSG, 0, NULL, NULL);
    if (recvCount == 0) {
        logError("Error receiving broadcast.", restaurant->name);
        return;
    }

    if (!strcmp(msgBuf, REG_REQ_MSG))
        broadcast(restaurant, serializerRestaurant(restaurant, REGISTERING));
    else {
        char* name;
        int port;
        BroadcastType type;
        deserializer(msgBuf, &name, &port, &type);
        if (!strcmp(restaurant->name, name) && restaurant->tcpPort != port) {
            int fd = connectServer(port);
            send(fd, TERMINATE_MSG, strlen(TERMINATE_MSG), 0);
            return;
        }
    }
}

void newConnectionHandler(int fd, Restaurant* restaurant, FdSet* fdset) {
    logInfo("New connection request.", restaurant->name);
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int newfd = accept(fd, (struct sockaddr*)&addr, &addrlen);
    if (newfd < 0) {
        logError("Error accepting new connection.", restaurant->name);
        return;
    }
    FD_SETTER(newfd, fdset);
    logInfo("New connection accepted.", restaurant->name);
}

void chatHandler(int fd, char* msgBuf, Restaurant* restaurant, FdSet* fdset) {
    int recvCount = recv(fd, msgBuf, BUF_MSG, 0);
    if (recvCount == 0) {
        logInfo("Connection closed.", restaurant->name);
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
