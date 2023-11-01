#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define BUF_MSG 512

int make_broad_sock() {
    int sock, broadcast = 1, opt = 1;
    char buffer[1024] = {0};
    struct sockaddr_in bc_address;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    return (sock);
}

struct sockaddr_in makeBroadAddr() {
    struct sockaddr_in bc_address;
    bc_address.sin_family = AF_INET;
    bc_address.sin_port = htons(8080);
    bc_address.sin_addr.s_addr = inet_addr("255.255.255.255");

    return (bc_address);
}

int setupServer(int port) {
    struct sockaddr_in address;
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));

    listen(server_fd, 4);

    return server_fd;
}

int acceptClient(int server_fd) {
    int client_fd;
    struct sockaddr_in client_address;
    int address_len = sizeof(client_address);
    client_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t *)&address_len);

    return client_fd;
}

int connectServer(unsigned short port) {
    int fd;
    struct sockaddr_in server_address;

    fd = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);        // ! FIXME
    server_address.sin_addr.s_addr = INADDR_ANY;  // STH TODO

    if (connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) <
        0) {  // checking for errors
        printf("Error in connecting to server\n");
    }

    return fd;
}

int main(int argc, char const *argv[]) {
    int udp_conn, tcp_conn, max_sd;
    char buffer[1024] = {0};
    struct sockaddr_in main_addr;

    fd_set rd_set, master_set;

    main_addr = makeBroadAddr();

    tcp_conn = setupServer(atoi(argv[1]));
    udp_conn = make_broad_sock();

    bind(udp_conn, (struct sockaddr *)&main_addr, sizeof(main_addr));

    FD_ZERO(&master_set);
    FD_SET(STDIN_FILENO, &master_set);
    FD_SET(tcp_conn, &master_set);
    FD_SET(udp_conn, &master_set);
    max_sd = udp_conn;

    while (1) {
        rd_set = master_set;

        select(max_sd + 1, &rd_set, NULL, NULL, NULL);

        if (FD_ISSET(0, &rd_set)) {
            memset(buffer, 0, 1024);
            read(0, buffer, 1024);

            char *cmd = strtok(buffer, " ");
            if (cmd == NULL) return 1;

            if (!strcmp(cmd, "connect")) {
                char *portStr = strtok(NULL, " ");
                int port = atoi(portStr);
                unsigned short usPort = (unsigned short)port;
                int newSocket = connectServer(usPort);
                char msg[BUF_MSG];
                sprintf(msg, "hello, I'm now connected to you. I can be found on port: %s\n",
                        argv[1]);
                write(1, msg, strlen(msg));
                send(newSocket, msg, strlen(msg), 0);
                if (newSocket > max_sd) max_sd = newSocket;
            } else {
                // int fd = (max_sd > 4) ? tcp_conn : udp_conn;
                sendto(udp_conn, buffer, strlen(buffer), 0, (struct sockaddr *)&main_addr,
                       sizeof(main_addr));
            }
        }

        else if (FD_ISSET(udp_conn, &rd_set)) {
            memset(buffer, 0, 1024);
            recv(udp_conn, buffer, 1024, 0);
            if (strcmp(buffer, "port\n") == 0) {
                memset(buffer, 0, 1024);
                sprintf(buffer, "%d", atoi(argv[1]));
                sendto(udp_conn, buffer, strlen(buffer), 0, (struct sockaddr *)&main_addr,
                       sizeof(main_addr));
            } else {
                printf("UDP: %s\n", buffer);
            }
        }

        else {
            for (int i = 0; i <= max_sd; i++) {
                if (FD_ISSET(i, &rd_set)) {
                    if (i == tcp_conn) {  // new clinet
                        char msg[BUF_MSG];
                        sprintf(msg, "in new client\n");
                        write(1, msg, strlen(msg));
                        int new_socket = acceptClient(tcp_conn);
                        FD_SET(new_socket, &master_set);
                        if (new_socket > max_sd) max_sd = new_socket;
                        sprintf(msg, "New client connected. fd = %d\n", new_socket);
                        write(1, msg, strlen(msg));
                    }

                    else {  // client sending msg
                        int bytes_received;
                        bytes_received = recv(i, buffer, 1024, 0);
                        if (bytes_received == 0) {  // EOF
                            char msg[BUF_MSG];
                            sprintf(msg, "client fd = %d closed\n", i);
                            write(1, msg, strlen(msg));
                            close(i);
                            FD_CLR(i, &master_set);
                            continue;
                        }
                        char msg[BUF_MSG];
                        sprintf(msg, "client %d: %s\n", i, buffer);
                        write(1, msg, strlen(msg));
                        memset(buffer, 0, 1024);
                    }
                }
            }
        }
    }

    return 0;
}
