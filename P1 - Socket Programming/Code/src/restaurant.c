#include "define.h"
#include "utils.h"

loadMenu(Restaurant* restaurant) {
    logInfo("Loading menu.");
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
    logInfo("Menu loaded.");
}

void printMenu(const Restaurant* restaurant) {
    logInfo("Printing menu.");
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
    logInfo("Menu printed.");
}

int initBroadcastRestaurant(Restaurant* restaurant) {
    logInfo("Initializing broadcast for restaurant.");
    // TODO: complete this
    logInfo("Broadcast for restaurant initialized.");
}

void initRestaurant(Restaurant* restaurant, char* port) {
    logInfo("Initializing restaurant.");
    initBroadcastRestaurant(restaurant);

    restaurant->tcpPort = atoi(port);
    initTCP(&restaurant->tcpPort);

    loadMenu(restaurant);
    logInfo("Restaurant initialized.");
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
