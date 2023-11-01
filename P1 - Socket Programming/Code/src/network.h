#ifndef NETWORK_H_INCLUDE
#define NETWORK_H_INCLUDE

#include "define.h"
#include "network.h"

struct sockaddr_in initBroadcastSockAddr();
int initBroadcast(struct sockaddr_in* addrOut);
int initTCP(unsigned short port);
int accClient(int socketId);
int connectServer(unsigned short port, int* outServerSocket);
void FD_SETTER(int socket, FdSet* fdset);
void FD_CLRER(int socket, FdSet* fdset);

#endif  // NETWORK_H_INCLUDE