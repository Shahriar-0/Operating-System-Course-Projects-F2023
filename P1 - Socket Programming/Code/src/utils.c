#include "utils.h"

////////////////////// Utils //////////////////////
void cliPrompt() { write(STDOUT_FILENO, ANSI_WHT ">> " ANSI_RST, 12); }

void errnoPrint() { perror(strerror(errno)); }

void printNum(int fd, int num) {
    char buffer[12] = {STRING_END};
    snprintf(buffer, 12, "%d", num);
    write(fd, buffer, strlen(buffer));
}

void getInput(int fd, const char* prompt, char* dst, size_t dstLen) {
    if (prompt != NULL) logInput(prompt);
    int cread = read(fd, dst, dstLen);
    if (cread <= 0) {
        errnoPrint();
        exit(EXIT_FAILURE);
    }
    dst[cread - 1] = STRING_END;
}

int strToInt(const char* str, int* res) {
    char* end;
    long num = strtol(str, &end, 10);

    if (*end != STRING_END) return 1;
    if (errno == ERANGE) return 2;

    *res = num;
    return 0;
}

int strToPort(const char* str, unsigned short* res) {
    int num;
    int ret = strToInt(str, &num);

    if (ret != 0) return ret;
    if (num < 0 || num > USHRT_MAX) return 2;

    *res = (unsigned short)num;
    return 0;
}

unsigned short strToPortErr(const char* str) {
    unsigned short port;
    int res = strToPort(str, &port);
    if (res == 1) {
        perror("Port should be a number.");
        exit(EXIT_FAILURE);
    } else if (res == 2) {
        perror("Port number (16-bit) out of range.");
        exit(EXIT_FAILURE);
    }
    return port;
}

int checkUnique(char* name, char names[MAX_TOTAL][BUF_NAME], int size) {
    for (int i = 0; i < size; i++)
        if (!strcmp(name, names[i])) return 0;
    return 1;
}

////////////////////// FdSet //////////////////////
void FD_SETTER(int socket, FdSet* fdset) {
    FD_SET(socket, &fdset->master);

    if (socket > fdset->max) fdset->max = socket;
}

void FD_CLRER(int socket, FdSet* fdset) {
    FD_CLR(socket, &fdset->master);

    if (socket == fdset->max) fdset->max--;
}

void InitFdSet(FdSet* fdset, int UDPfd) {
    fdset->max = 0;
    FD_ZERO(&fdset->master);
    FD_ZERO(&fdset->working);
    FD_SETTER(STDIN_FILENO, fdset);
    FD_SETTER(UDPfd, fdset);
}

////////////////////// JSON //////////////////////
char* read_file(const char* filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("error in open file");
        return NULL;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("Cannot get file size");
        close(fd);
        return NULL;
    }

    char* buffer = malloc(st.st_size + 1);
    if (buffer == NULL) {
        perror("Cannot allocate buffer");
        close(fd);
        return NULL;
    }

    ssize_t bytes_read = read(fd, buffer, st.st_size);
    if (bytes_read != st.st_size) {
        perror("Cannot read file");
        free(buffer);
        close(fd);
        return NULL;
    }

    buffer[st.st_size] = STRING_END;

    close(fd);
    return buffer;
}

cJSON* loadJSON() {
    char* jsonAddress = RECIPE_ADDRESS;
    char* json = read_file(jsonAddress);
    cJSON* root = cJSON_Parse(json);
    if (root == NULL) {
        char* error_ptr = cJSON_GetErrorPtr();
        char errmsg[BUF_MSG] = {STRING_END};
        sprintf(errmsg, "Error before: %s\n", error_ptr);
        perror(errmsg);
        return;
    }
    return root;
}

int deserializer(char* msg, char** name, int* port, BroadcastType* type) {
    char* token = strtok(msg, BCAST_IN_DELIM);
    if (token == NULL) return;
    RegisteringState state = atoi(token);
    token = strtok(NULL, BCAST_IN_DELIM);
    if (token == NULL) return;
    *name = token;
    token = strtok(NULL, BCAST_IN_DELIM);
    if (token == NULL) return;
    port = atoi(token);
    token = strtok(NULL, BCAST_OUT_DELIM);
    if (token == NULL) return;
    *type = atoi(token);

    return state == REGISTERING;
}

void yesNoPromptSupplier(char* name, unsigned short port) {
    char msg[BUF_MSG] = {STRING_END};

    char ans[BUF_MSG];
    getInput(STDIN_FILENO, "Accept? (y/n)", ans, BUF_MSG);

    int ansFd = connectServer(port);

    if (!strcmp(ans, "y")) {
        snprintf(msg, BUF_MSG, "accepted by %s.", name);
        logInfo(msg, name);
        send(ansFd, ACCEPTED_MSG, strlen(ACCEPTED_MSG), 0);
    } else if (!strcmp(ans, "n")) {
        snprintf(msg, BUF_MSG, "rejected by %s.", name);
        logInfo(msg, name);
        send(ansFd, REJECTED_MSG, strlen(REJECTED_MSG), 0);
    } else {
        logError("Invalid answer.", name);
    }
}

// state | name | tcpPort | type
char* serializerSupplier(Supplier* supplier, RegisteringState state) {
    char broadMsg[BUF_MSG] = {STRING_END};

    // clang-format off
    snprintf(broadMsg, BUF_MSG, "%d%c%s%c%d%c%d%c", 
             state, BCAST_IN_DELIM, 
             supplier->name, BCAST_IN_DELIM, 
             supplier->tcpPort, BCAST_IN_DELIM, 
             SUPPLIER, BCAST_OUT_DELIM);
    // clang-format on

    return strdup(broadMsg);
}

char* serializerCustomer(Customer* customer, RegisteringState state) {
    char broadMsg[BUF_MSG] = {STRING_END};

    // clang-format off
    snprintf(broadMsg, BUF_MSG, "%d%c%s%c%d%c%d%c", 
             state, BCAST_IN_DELIM, 
             customer->name, BCAST_IN_DELIM, 
             customer->tcpPort, BCAST_IN_DELIM, 
             CUSTOMER, BCAST_OUT_DELIM);
    // clang-format on

    return strdup(broadMsg);
}

char* serializerRestaurant(Restaurant* restaurant, RegisteringState state) {
    char broadMsg[BUF_MSG] = {STRING_END};

    // clang-format off
    snprintf(broadMsg, BUF_MSG, "%d%c%s%c%d%c%d%c", 
             state, BCAST_IN_DELIM, 
             restaurant->name, BCAST_IN_DELIM, 
             restaurant->tcpPort, BCAST_IN_DELIM, 
             RESTAURANT, BCAST_OUT_DELIM);
    // clang-format on

    return strdup(broadMsg);
}


void exitall(char* name) {
    logInfo("Moving log file.", name);
    char dst[BUF_MSG] = {STRING_END};
    char src[BUF_MSG] = {STRING_END};
    sprintf(src, "%s%s%s", LOG_FOLDER_ADD, name, LOG_EXT);
    sprintf(dst, "%s%s%s", LOG_OVER_ADD, name, LOG_EXT);
    
    if (rename(src, dst)) 
        perror("Error moving log file");
    
    if (remove(src) == 0) 
        perror("Failed to delete the file.");

    write(STDOUT_FILENO, "Exited successfully.\n", 21);
    exit(EXIT_SUCCESS);
}