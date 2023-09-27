#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_BROADCAST "127.255.255.255"
#define BUFFER_SIZE 1024
#define RECIEVE_ERROR "Error On Reading From the Server"
#define CHAT_PATTERN "$$$"
#define SPECT_PATTERN "&&&"
#define EXIT_PAT "exit\n"
#define SO_REUSEPORT 15
#define EXIT_SIG "$@&^()+$#"
#define ALARM_PATTERN "@*alarm*@"
#define TA_TIMEOUT 60
#define NO_RESPONSE "&NO$REsp*"
#define NO_RES_MSG "No Response!\nexiting...\n"

int connect_server(int port);

void recieved_handler(char* buffer, int num_of_bytes);

int port_pattern(const char* buffer);

void room_communication(int port, int serverFD);

void spect_opt(int port);

static int serFD = 0;

static int flag = 0;

static int DEFAULT_SERVER_PORT;

void my_handler(int sig)
{
    send(serFD, NO_RESPONSE, strlen(NO_RESPONSE), 0);
    write(1, NO_RES_MSG, strlen(NO_RES_MSG));
    exit(1);
}

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        perror("bad argument");
        exit(1);
    }
    DEFAULT_SERVER_PORT = atoi(argv[1]);
    char buffer[BUFFER_SIZE] = {0};
    int client_fd = connect_server(DEFAULT_SERVER_PORT);
    serFD = client_fd;
    alarm(0);
    signal(SIGALRM, my_handler);
    fd_set temp, master_set;
    FD_ZERO(&master_set);
    FD_SET(0, &master_set);
    FD_SET(client_fd, &master_set);
    while(1)
    {
        temp = master_set;
        select(client_fd+1, &temp, NULL, NULL, NULL);
        if(FD_ISSET(0, &temp))
        {
            memset(buffer, 0, sizeof(buffer)/sizeof(buffer[0]));
            read(0, buffer, BUFFER_SIZE); // Read client inp and Send to Server
            alarm(0);
            if(flag)
            {
                alarm(TA_TIMEOUT);
            }
            send(client_fd, buffer, strlen(buffer), 0);
        }
        else if(FD_ISSET(3, &temp))
        {
            // receive from server
            int recieved_bytes = recv(3, buffer, BUFFER_SIZE, 0);
            recieved_handler(buffer, recieved_bytes);
        }
        memset(buffer, 0, (size_t)sizeof(buffer)/sizeof(buffer[0]));
    }
}

int connect_server(int port)
{
    int fd;
    struct sockaddr_in server_address;
    int opt = 1;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET; 
    server_address.sin_port = htons(port); 
    server_address.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    if (connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) { // checking for errors
        printf("Error in connecting to server\n");
    }
    
    return fd;
}

void recieved_handler(char* buffer, int num_of_bytes)
{
    if(num_of_bytes == 0)
    {
        return;
    }
    else if(num_of_bytes > 0)
    {
        int port = port_pattern(buffer);
        if(strncmp(ALARM_PATTERN, buffer, strlen(ALARM_PATTERN))
            == 0)
        {
            alarm(0);
            alarm(TA_TIMEOUT);
            flag = 1;
            return;
        }
        if(port <= 0)
        {
            write(1, buffer, num_of_bytes);
            return;
        }
        else
        {
            // inefficient implementation but don't have time
            if(strncmp(buffer, CHAT_PATTERN, strlen(CHAT_PATTERN))
             == 0)
            {
                room_communication(port, 3);
                return;
            }
            else
            {
                spect_opt(port);
                return;
            }   
        }
    }
    else 
    {
        perror(RECIEVE_ERROR);
        int log = open("log.txt", O_APPEND, O_CREAT);
        write(log, RECIEVE_ERROR, strlen(RECIEVE_ERROR));
        close(log);
        return;
    }
}

int port_pattern(const char* buffer)
{
    if(strncmp(buffer, CHAT_PATTERN,
         strlen(CHAT_PATTERN)) == 0)
    {
        char temp[BUFFER_SIZE];
        strcpy(temp, buffer);
        char* token = strtok(temp, CHAT_PATTERN);
        int port = atoi(token);
        return port;
    }
    if(strncmp(buffer, SPECT_PATTERN, strlen(SPECT_PATTERN))
        == 0)
    {
        char stemp[BUFFER_SIZE];
        strcpy(stemp, buffer);
        char* stoken = strtok(stemp, SPECT_PATTERN);
        int spport = atoi(stoken); 
        return spport;
    }
    if(strncmp(EXIT_SIG, buffer, strlen(EXIT_SIG)) == 0)
    {
        exit(1);
    }
    return 0;
}

void room_communication(int port, int serverFD)
{
    struct sockaddr_in sock_adr;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket < 0)
    {
        perror("error socket");
    }
    // broadcast specs:
    sock_adr.sin_port = htons(port);
    sock_adr.sin_family = AF_INET;
    sock_adr.sin_addr.s_addr = inet_addr(SERVER_BROADCAST);
    int broadcast = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast,
        sizeof(broadcast)) < 0)
    {
            perror("error setsocketopt broadcast");
    }
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &broadcast,
        sizeof(broadcast)) < 0)
    {
        perror("error setsocket reuse");
    }
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &broadcast,
        sizeof(broadcast)) < 0)
    {
        perror("error setsocket reuse");
    }
    if(bind(sock, (const struct sockaddr*) &sock_adr, 
        (socklen_t)sizeof(sock_adr)) < 0)
    {
        perror("error binding");
    }
    char b_buf[BUFFER_SIZE];
    int sock_adr_len = sizeof(sock_adr);
    memset(b_buf, 0, BUFFER_SIZE);
    fd_set temp, master;
    FD_ZERO(&master);
    FD_SET(0, &master);
    FD_SET(sock, &master);
    FD_SET(serverFD, &master);
    if(flag) // inefficient implementation
    {
        alarm(TA_TIMEOUT);
    }
    while(1)
    {
        memset(b_buf, 0, BUFFER_SIZE);
        temp = master;
        if(select(sock+1, &temp, NULL, 
            NULL, NULL) < 0)
        {
            perror("socket error");
        }
        if(FD_ISSET(0, &temp)) // read triggered
        {
            read(0, b_buf, BUFFER_SIZE);
            alarm(0);
            if(flag)
            {
                alarm(TA_TIMEOUT);
            }
            if(strcmp(b_buf, EXIT_PAT) == 0)
            {
                send(serverFD, EXIT_PAT, strlen(EXIT_PAT), 0); // trigger server
                return;
            }
            // int sch = sendto(serverFD, b_buf, strlen(b_buf), 0, // send to server instead of broadcasting in the first place causing problems
            //  (const struct sockaddr*)&sock_adr,
            //     sizeof(sock_adr));
            int sch = send(serverFD, b_buf, strlen(b_buf), 0);
            if(sch <= 0)
            {
                perror("send problem");
            }
        }
        else if(FD_ISSET(sock, &temp)) // broadcast triggered
        {
            int rch = recvfrom(sock, b_buf, BUFFER_SIZE, 0,
            (struct sockaddr*) &sock_adr, (socklen_t*)&sock_adr_len);
            if(rch <= 0)
            {
                perror("recv error");
            }
            if(strncmp(NO_RESPONSE, b_buf, strlen(NO_RESPONSE)) == 0)
            {
                return;
            }
            write(1, b_buf, strlen(b_buf));
        }
        else
        {
            perror("uknown desc");
            return;
        }
    }
}   

void spect_opt(int port)
{   
    // setting up socket should've been a separate function
    // no time though
    struct sockaddr_in sock_adr;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket < 0)
    {
        perror("error socket");
    }
    // broadcast specs:
    sock_adr.sin_port = htons(port);
    sock_adr.sin_family = AF_INET;
    sock_adr.sin_addr.s_addr = inet_addr(SERVER_BROADCAST);
    int broadcast = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast,
        sizeof(broadcast)) < 0)
    {
        perror("error setsocketopt broadcast");
    }
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &broadcast,
        sizeof(broadcast)) < 0)
    {
        perror("error setsocket reuse");
    }
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &broadcast,
        sizeof(broadcast)) < 0)
    {
        perror("error setsocket reuse");
    }
    if(bind(sock, (const struct sockaddr*) &sock_adr, 
        (socklen_t)sizeof(sock_adr)) < 0)
    {
        perror("error binding");
    }
    char b_buf[BUFFER_SIZE];
    int sock_adr_len = sizeof(sock_adr);
    memset(b_buf, 0, BUFFER_SIZE);
    while(1)
    {
        int bytes = recvfrom(sock, b_buf, BUFFER_SIZE, 0
        , (struct sockaddr*)&sock_adr, (socklen_t*)&sock_adr_len);
        if(bytes <= 0)
        {
            perror("recv error");
        }
        write(1, b_buf, strlen(b_buf));
        memset(b_buf, 0, BUFFER_SIZE);
    }
}
