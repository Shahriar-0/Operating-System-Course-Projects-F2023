#ifndef DEFINE_H_INCLUDE
#define DEFINE_H_INCLUDE

// includes
#include <arpa/inet.h>
#include <cjson/cJSON.h>
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
#define _XOPEN_SOURCE   700
#define LAMINATION      "----------------------------------------"
#define LAMLEN          40
#define BUF_NAME        32
#define BUF_PNAME       64
#define BUF_CLI         128
#define BUF_MSG         512
#define BCAST_IP        "255.255.255.255"
#define TIMEOUT         90
#define RECIPE_ADDRESS  "../../../Assets/recipes.json"
#define LOG_FOLDER_ADD  "../../log/"
#define LOG_OVER_ADD    "../../log/over/"
#define MAX_INGREDIENT  20
#define MAX_SUPPLIER    20
#define MAX_RESTAURANT  20
#define MAX_TOTAL       20
#define MAX_FOOD        20
#define MAX_REQUEST     20
#define MAX_LISTEN      4
#define UDP_PORT        8080
#define LOCAL_HOST      "127.0.0.1"
#define STRING_END      '\0'
#define BCAST_IN_DELIM  "|"
#define BCAST_OUT_DELIM "-"
#define REQ_IN_DELIM    "|"
#define REJECTED_MSG    "REJECTED"
#define ACCEPTED_MSG    "ACCEPTED"
#define REG_REQ_MSG     "REG_REQ"
#define TERMINATE_MSG   "TERMINATE"
#define CLOSE_REST_MSG  "CLOSE_REST"
#define OPEN_REST_MSG   "OPEN_REST"
#define REQ_DELIM       ":"
#define NAME_DELIM      "-"
#define LOG_EXT         ".log"
// clang-format on

// types and states
typedef enum { REGISTERING, NOT_REGISTERING } RegisteringState;
typedef enum { CUSTOMER, RESTAURANT, SUPPLIER } BroadcastType;

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
    char customerName[BUF_NAME];
    char foodName[BUF_NAME];
    int customerPort;
} FoodRequest;

typedef enum { OPEN, CLOSED } RestaurantState;
typedef struct {
    char name[BUF_NAME];
    unsigned short tcpPort;
    RestaurantState state;
    BroadcastInfo bcast;
    BroadcastData suppliers[MAX_SUPPLIER];
    int supplierSize;
    Food menu[MAX_FOOD];
    int menuSize;
    Ingredient ingredients[MAX_INGREDIENT];
    int ingredientSize;
    FoodRequest pendingRequests[MAX_REQUEST];
    int pendingRequestSize;
    FoodRequest handledRequests[MAX_REQUEST];
    int handledRequestsSize;
} Restaurant;

typedef struct {
    char name[BUF_NAME];
    unsigned short tcpPort;
    BroadcastInfo bcast;
    char* foods[MAX_FOOD];
    int foodSize;
    BroadcastData restaurants[MAX_RESTAURANT];
    int restaurantSize;
} Customer;

typedef struct {
    char name[BUF_PNAME];
    unsigned short tcpPort;
    BroadcastInfo bcast;
} Supplier;
#endif  // DEFINE_H_INCLUDE
