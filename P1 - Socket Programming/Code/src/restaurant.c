#include "define.h"
#include "logger.h"

void printMenu(const Restaurant* restaurant) {
    logNormal("Menu:\n");
    for (int i = 0; i < restaurant->menuSize; i++) {
        const Food* food = &restaurant->menu[i];
        char buf[BUF_MSG] = {STRING_END};
        sprintf(buf, "%d. %s:", i + 1, food->name);
        logNormal(buf);
        for (int j = 0; j < food->ingredientSize; j++) {
            memset(buf, STRING_END, BUF_MSG);
            sprintf(buf, "     - %s: %d\n", food->ingredients[j], food->quantity[j]);
        }
        logNormal("\n");
    }
}

void initRestaurant(Restaurant* restaurant, char* port) {
    restaurant->tcpPort = atoi(port);
    initTCP(&restaurant->tcpPort);

    loadMenu(restaurant);
    printMenu(restaurant); // * for testing
}

int main(int argc, char** argv) {
    if (argc != 2) {
        logError("Usage: ./supplier <port>");
        exit(EXIT_FAILURE);
    }

    Restaurant restaurant;
    restaurant.tcpPort = atoi(argv[1]);
    initRestaurant(&restaurant, argv[1]);
}
