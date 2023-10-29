#include "define.h"
#include "network.h"


int initBroadcastCustomer(Customer* customer) {
    int bcfd = initBroadcast(&customer->bcast.addr);
    if (bcfd < 0) return bcfd;
    customer->bcast.fd = bcfd;
    
    // getting into of all restaurants
    char* msg = REG_MSG;
    // TODO: check for uniqueness among customers
}