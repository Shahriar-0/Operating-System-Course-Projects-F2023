#ifndef NETWORK_H_INCLUDE
#define NETWORK_H_INCLUDE

#include "define.h"


int initBroadcast(struct sockaddr_in* addrOut);
struct sockaddr_in initBroadcastSockAddr();

#endif // NETWORK_H_INCLUDE