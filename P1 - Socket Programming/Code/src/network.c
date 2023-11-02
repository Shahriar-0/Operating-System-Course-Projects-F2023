#include "network.h"

struct sockaddr_in initBroadcastSockAddr() {
    struct sockaddr_in bcAddress;
    bcAddress.sin_family = AF_INET;
    bcAddress.sin_port = htons(UDP_PORT);
    bcAddress.sin_addr.s_addr = inet_addr(BCAST_IP);
    memset(bcAddress.sin_zero, STRING_END, sizeof(bcAddress.sin_zero));
    return bcAddress;
}

int initBroadcast(struct sockaddr_in* addrOut) {
    int socketId, broadcast = 1, opt = 1;
    char buffer[BUF_MSG] = {0};
    struct sockaddr_in bcAddress = initBroadcastSockAddr();

    socketId = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketId < 0) return socketId;
    setsockopt(socketId, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(socketId, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bind(socketId, (struct sockaddr*)&bcAddress, sizeof(bcAddress));

    *addrOut = bcAddress;

    return socketId;
}

int setupSocket(unsigned short port, struct sockaddr_in* addr) {
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) return serverFd;

    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = INADDR_ANY;
    addr->sin_port = htons(port);
    memset(addr->sin_zero, STRING_END, sizeof(addr->sin_zero));

    return serverFd;
}

int initTCP(unsigned short port) {

    int serverFd;
    struct sockaddr_in addr;
    setupSocket(port, &addr);
  
    bind(serverFd, (struct sockaddr*)&addr, sizeof(addr));
    listen(serverFd, MAX_LISTEN);

    return serverFd;
}

int accClient(int socketId) {
    struct sockaddr_in clientAddr;
    int addrSize = sizeof(clientAddr);
    int clientSocket = accept(socketId, (struct sockaddr*)&clientAddr, (socklen_t*)&addrSize);
    if (clientSocket < 0) return clientSocket;
    return clientSocket;
}

int connectServer(unsigned short port) {
    int serverId = socket(PF_INET, SOCK_STREAM, 0);
    if (serverId < 0) return serverId;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(BCAST_IP);
    memset(addr.sin_zero, STRING_END, sizeof(addr.sin_zero));

    int res = connect(serverId, (struct sockaddr*)&addr, sizeof(addr));
    if (res < 0) return res;

    return serverId;
}

