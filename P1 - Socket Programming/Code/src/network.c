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
    addr->sin_port = htons(port);
    addr->sin_addr.s_addr = INADDR_ANY;
    memset(addr->sin_zero, STRING_END, sizeof(addr->sin_zero));

    return serverFd;
}

int initTCP(unsigned short port) {
    int serverFd;
    struct sockaddr_in addr;
    serverFd = setupSocket(port, &addr);

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
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (connect(fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) 
        perror("Error connecting to server");

    return fd;
}
