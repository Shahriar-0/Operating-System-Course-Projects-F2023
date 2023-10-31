#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

// #include "udp.h"
// #include "tcp.h"
// #include "tcp.c"
// #include "udp.c"
// #include "consts.c"

#define LOG_BUF_SIZE 1024

typedef struct ParsedLine {
    int argc;
    char** argv;
} ParsedLine;

const char* INVALID_COMMAND_FORMAT =
    "Invalid command format:\n"
    "   \"%s\" requires %d number of arg(s), You entered %d arg(s).\n";

const char* UNKNOWN_COMMAND = "Unknown command.\n";

const char* GETADDR_FAILED = "getaddrinfo failed.\n";

const char* SOCKET_ERR = "socket failed.\n";

const char* BIND_ERR = "bind failed.\n";

const char* UDP_BIND_FAILED = "udp client failed to bind socket.\n";

const char* UDP_RECV_FAILED = "recieving udp message failed.\n";

const char* BROADCAST_FAILED = "sending broadcast failed.\n";

const char* INVALID_AD_OFFER = "selected ad id is either in porgress, expired or doesn't exist.\n";

const char* INVALID_OFFER_ID = "offer not found.\n";

const char* ACCEPT_OFFER_MSG = "/accept/";

const char* DECLINE_OFFER_MSG = "/decline/";

const char* WAITING_STAT_STR = "waiting";

const char* IN_PROG_STAT_STR = "in progress";

const char* EXPIRED_STAT_STR = "expired";

const char* TIME_OUT_MSG = "time out";

char* read_line(int fd, int limit) {
    char* result = NULL;
    int curr_size = 0;
    for (; limit != 0; --limit) {
        ++curr_size;
        result = (char*)realloc(result, curr_size);
        read(fd, &result[curr_size - 1], 1);
        if (result[curr_size - 1] == '\n') {
            result[curr_size - 1] = '\0';
            break;
        }
    }
    return result;
}

ParsedLine parse_line(char* input_line, const char* delims) {
    ParsedLine result = {0, NULL};
    int input_line_len = strlen(input_line);
    int last_tok_end = 0;
    for (int i = 0; i <= input_line_len; ++i) {
        if (strchr(delims, input_line[i]) != NULL) {
            result.argv = (char**)realloc(result.argv, result.argc + 1);
            result.argv[result.argc] = (char*)calloc(i - last_tok_end + 1, sizeof(char));
            memcpy(result.argv[result.argc], &input_line[last_tok_end], i - last_tok_end);
            result.argv[result.argc][i - last_tok_end] = '\0';
            ++result.argc;
            last_tok_end = i + 1;
        }
    }
    return result;
}

// void put_time_header(char* dest) {
//     time_t t = time(NULL);
//     struct tm* tm = localtime(&t);
//     sprintf(
//         dest,
//         "%d-%d-%d _ %d:%d:%d",
//         tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
// }

// void log_info(int fd, const char* msg, int include_time, int include_tag) {
//     char buf[LOG_BUF_SIZE] = {0};
//     if (include_time)
//         put_time_header(buf);
//     strcat(buf, "\n");
//     if (include_tag)
//         strcat(buf, "[INFO] ");
//     strcat(buf, msg);
//     write(fd, buf, strlen(buf));
// }

// void log_err(int fd, const char* msg, int include_time, int include_tag) {
//     char buf[LOG_BUF_SIZE] = {0};
//     if (include_time)
//         put_time_header(buf);
//     strcat(buf, "\n");
//     if (include_tag)
//         strcat(buf, "[ERROR] ");
//     strcat(buf, msg);
//     write(fd, buf, strlen(buf));
// }

char* getline_str(const char* str) {
    char* occ;
    if ((occ = (char*)strchr(str, '\n')) == NULL) occ = strchr(str, '\0');

    char* result = (char*)calloc((size_t)(occ - str), sizeof(char));
    for (int i = 0; i + str < occ; ++i) result[i] = str[i];
    return result;
}

char* add_data_header(char* data, const char* header) {
    char* result;
    int result_len = strlen(data) + strlen(header);
    result = (char*)calloc(result_len, sizeof(char));
    strcpy(result, header);
    strcat(result, data);
    return result;
}

char* indent_str(const char* str, char indent_char, int indent_level) {
    int line_count = 1;
    int str_len = strlen(str);

    for (int i = 0; i < str_len; ++i)
        if (str[i] == '\n') ++line_count;

    const char* current_line_end = str;
    char* result = (char*)calloc(str_len + (indent_level * line_count), sizeof(char));
    int current_len = 0;

    for (; line_count > 0; --line_count) {
        for (int i = 0; i < indent_level; ++i) result[current_len + i] = indent_char;

        char* current_line = getline_str(current_line_end);
        strcat(result, current_line);
        current_line_end += strlen(current_line);
        free(current_line);
        current_len += strlen(result);
    }
    return result;
}

char* get_field_val(const char* str, const char* field) {
    char* result = NULL;
    char* field_begin;
    if ((field_begin = strstr(str, field)) != NULL)
        result = getline_str(field_begin + strlen(field));
    return result;
}

// const char* INVALID_COMMAND_FORMAT =
//     "Invalid command format:\n"
//     "   \"%s\" requires %d number of arg(s), You entered %d arg(s).\n";

// const char* UNKNOWN_COMMAND =
//     "Unknown command.\n";

// const char* GETADDR_FAILED =
//     "getaddrinfo failed.\n";

// const char* SOCKET_ERR =
//     "socket failed.\n";

// const char* BIND_ERR =
//     "bind failed.\n";

// const char* UDP_BIND_FAILED =
//     "udp client failed to bind socket.\n";

// const char* UDP_RECV_FAILED =
//     "recieving udp message failed.\n";

// const char* BROADCAST_FAILED =
//     "sending broadcast failed.\n";

// const char* INVALID_AD_OFFER =
//     "selected ad id is either in porgress, expired or doesn't exist.\n";

// const char* INVALID_OFFER_ID =
//     "offer not found.\n";

// const char* ACCEPT_OFFER_MSG =
//     "/accept/";

// const char* DECLINE_OFFER_MSG =
//     "/decline/";

// const char* WAITING_STAT_STR =
//     "waiting";

// const char* IN_PROG_STAT_STR =
//     "in progress";

// const char* EXPIRED_STAT_STR =
//     "expired";

// const char* TIME_OUT_MSG =
//     "time out";

void broadcast_msg(const char* port, const char* msg) {
    struct sockaddr_in bc_address;
    int broadcast = 1, reuse_port = 1;

    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &reuse_port, sizeof(reuse_port));
    bc_address.sin_family = AF_INET;
    bc_address.sin_port = htons(8082);
    bc_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    bind(sock_fd, (struct sockaddr*)&bc_address, sizeof(bc_address));

    if (sendto(sock_fd, msg, strlen(msg), 0, (struct sockaddr*)&bc_address, sizeof(bc_address)) ==
        -1) {
        // log_err(STDERR_FILENO, BROADCAST_FAILED, 1, 1);
        exit(EXIT_FAILURE);
    }
    close(sock_fd);
}

int connect_udp(const int port) {
    struct sockaddr_in bc_address;
    int broadcast = 1, reuse_port = 1;

    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &reuse_port, sizeof(reuse_port));
    bc_address.sin_family = AF_INET;
    bc_address.sin_port = htons(port);
    bc_address.sin_addr.s_addr = inet_addr("255.255.255.255");

    bind(sock_fd, (struct sockaddr*)&bc_address, sizeof(bc_address));

    return sock_fd;
}

char* rcv_udp(int sock_fd) {
    char buf[1024] = {0};

    char* result = NULL;
    int total_bytes_rcvd = 0;

    for (;;) {
        int recv_bytes = recv(sock_fd, buf, 1024, 0);
        if (recv_bytes <= 0) return result;
        result = (char*)realloc(result, recv_bytes);
        memcpy(result, buf, recv_bytes);
        total_bytes_rcvd += recv_bytes;
        if (recv_bytes == 1024) {
            fd_set read_fd_set;
            FD_ZERO(&read_fd_set);
            FD_SET(sock_fd, &read_fd_set);
            struct timeval t = {0, 0};
            select(sock_fd, &read_fd_set, NULL, NULL, &t);
            if (FD_ISSET(sock_fd, &read_fd_set)) continue;
        }
        break;
    }
    result = (char*)realloc(result, total_bytes_rcvd + 1);
    result[total_bytes_rcvd] = '\0';

    return result;
}

int connect_tcp_server(int port) {
    struct sockaddr_in address;
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        close(server_fd);
        return -1;
    }

    listen(server_fd, 4);

    return server_fd;
}

int accept_tcp_client(int server_fd) {
    int client_fd;
    struct sockaddr_in client_address;
    int address_len = sizeof(client_address);

    client_fd = accept(server_fd, (struct sockaddr*)&client_address, (socklen_t*)&address_len);

    return client_fd;
}

int connect_tcp_client(int port) {
    int fd;
    struct sockaddr_in server_address;

    fd = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(fd, (struct sockaddr*)&server_address, sizeof(server_address)) <
        0) {  // checking for errors
        printf("Error in connecting to server\n");
    }

    return fd;
}

int send_tcp_msg(int sock_fd, const char* msg, int max_tries) {
    int left_bytes = strlen(msg);
    int tries = 0;
    for (; tries <= max_tries && left_bytes > 0; ++tries) {
        int sent_bytes = send(sock_fd, msg, left_bytes, 0);
        if (sent_bytes == -1) {
            // log_err(STDERR_FILENO, "tcp message send failed.\n", 1, 1);
            return -1;
        }
        left_bytes -= sent_bytes;
    }
    // if (left_bytes > 0)
    //     log_err(STDERR_FILENO, "couldn't send tcp message completly.\n", 1, 1);

    return left_bytes;
}

char* receive_tcp(int sock_fd) {
    int num_bytes = 0, cur_size = 0, total_size = 0;
    char buf[1024] = {0};
    char* result = NULL;
    for (;;) {
        num_bytes = recv(sock_fd, buf, 1024, 0);
        if (num_bytes <= 0) return result;
        total_size += num_bytes;
        result = (char*)realloc(result, cur_size + num_bytes);
        memcpy(&result[cur_size], buf, num_bytes);
        cur_size += num_bytes;
        if (num_bytes < 1024) break;
        num_bytes = 0;
    }
    result = (char*)realloc(result, total_size + 1);
    result[total_size] = '\0';
    return result;
}

struct sockaddr_in makeBroadAddr() {
    struct sockaddr_in bc_address;
    bc_address.sin_family = AF_INET;
    bc_address.sin_port = htons(8082);
    bc_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    return (bc_address);
}
//
void interface(char* arg) {
    int udp_connection, tcp_connection, max_sd;
    char buffer[1024] = {0};
    struct sockaddr_in main_addr;

    fd_set rd_set, master_set;

    main_addr = makeBroadAddr();

    tcp_connection = connect_tcp_server(atoi(arg));
    udp_connection = connect_udp(atoi(arg));

    bind(udp_connection, (struct sockaddr*)&main_addr, sizeof(main_addr));

    max_sd = udp_connection;
    FD_ZERO(&master_set);
    FD_SET(STDIN_FILENO, &master_set);
    FD_SET(udp_connection, &master_set);

    while (1) {
        rd_set = master_set;

        select(max_sd + 1, &rd_set, NULL, NULL, NULL);

        if (FD_ISSET(0, &rd_set)) {
            memset(buffer, 0, 1024);
            read(0, buffer, 1024);
            sendto(udp_connection, buffer, strlen(buffer), 0, (struct sockaddr*)&main_addr,
                   sizeof(main_addr));
        }

        else if (FD_ISSET(udp_connection, &rd_set)) {
            memset(buffer, 0, 1024);
            recv(udp_connection, buffer, 1024, 0);
            if (strcmp(buffer, "port\n") == 0) {
                memset(buffer, 0, 1024);
                sprintf(buffer, "%d", atoi(arg));
                sendto(udp_connection, buffer, strlen(buffer), 0, (struct sockaddr*)&main_addr,
                       sizeof(main_addr));
            } else {
                printf("UDP: %s\n", buffer);
            }
        }

        else {
            for (int i = 0; i <= max_sd; i++) {
                if (FD_ISSET(i, &rd_set)) {
                    if (i == tcp_connection) {  // new clinet
                        int new_socket = accept_tcp_client(tcp_connection);
                        FD_SET(new_socket, &master_set);
                        if (new_socket > max_sd) max_sd = new_socket;
                        printf("New client connected. fd = %d\n", new_socket);
                    }

                    else {  // client sending msg
                        int bytes_received;
                        bytes_received = recv(i, buffer, 1024, 0);

                        if (bytes_received == 0) {  // EOF
                            printf("client fd = %d closed\n", i);
                            close(i);
                            FD_CLR(i, &master_set);
                            continue;
                        }

                        printf("client %d: %s\n", i, buffer);
                        memset(buffer, 0, 1024);
                    }
                }
            }
        }
    }

    // return 0;
}

int main(int argc, char const* argv[]) {

    interface("8082");
    return 0;
}