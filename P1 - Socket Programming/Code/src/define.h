#ifndef DEFINE_H_INCLUDE
#define DEFINE_H_INCLUDE

// includes
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

// Constants
// clang-format off
#define BUF_NAME       32
#define BUF_PNAME      64
#define BUF_CLI        128
#define BUF_MSG        512
#define BCAST_IP       "255.255.255.255"
#define TIMEOUT        90
#define RECIPE_ADDRESS "../../../Assets/recipes.json"
#define MAX_INGREDIENT 20
#define MAX_SUPPLIER   20
#define MAX_RESTAURANT 20
#define MAX_FOOD       20
#define MAX_REQUEST    20
#define MAX_LISTEN     4
#define UDP_PORT       8080
#define UDP_IP         "255.255.255.255"
#define LOCAL_HOST     "127.0.0.1"
#define STRING_END     '\0'
#define REG_MSG        "$REG$"
// clang-format on

// Structs
typedef char* Ingredient;

typedef struct {
    char name[BUF_NAME];
    Ingredient ingredients[MAX_INGREDIENT];
    int quantity[MAX_INGREDIENT];
    int ingredientSize;
} Food;

typedef struct {
    int fd;
    struct sockaddr_in addr;
} BroadcastInfo;

typedef struct {
    char name[BUF_NAME];
    unsigned short port;
} BroadcastData;

typedef struct {
    int max;
    fd_set master;
    fd_set working;
} FdSet;

typedef struct {
    char name[BUF_PNAME];
    BroadcastInfo bcast;
    unsigned short tcpPort;
} Supplier;

typedef enum { OPEN = 0, CLOSED = 1 } RestaurantState;

typedef struct {
    char customerName[BUF_NAME];
    int customerPort;
    char foodName[BUF_NAME];
} FoodRequest;

typedef struct {
    char name[BUF_NAME];
    BroadcastData suppliers[MAX_SUPPLIER];
    int supplierSize;
    Food menu[MAX_FOOD];
    int menuSize;
    Ingredient ingredients[MAX_INGREDIENT];
    int ingredientSize;
    RestaurantState state;
    FoodRequest pendingRequest[MAX_REQUEST];
    int pendingRequestSize;
    FoodRequest handledRequests[MAX_REQUEST];
    int handledRequestsSize;
    unsigned short tcpPort;
    BroadcastInfo bcast;
} Restaurant;

typedef struct {
    char name[BUF_NAME];
    Food foods[MAX_FOOD];
    BroadcastData restaurants[MAX_RESTAURANT];
    int restaurantSize;
    BroadcastInfo bcast;
} Customer;

#endif  // DEFINE_H_INCLUDE
