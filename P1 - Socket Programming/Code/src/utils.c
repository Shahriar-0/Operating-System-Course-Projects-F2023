#include "utils.h"

////////////////////// Utils //////////////////////
void cliPrompt() { write(STDOUT_FILENO, ANSI_WHT ">> " ANSI_RST, 12); }

void errnoPrint() { perror(strerror(errno)); }

void getInput(int fd, const char* prompt, char* dst, size_t dstLen) {
    if (prompt != NULL) logInput(prompt);
    int cread = read(fd, dst, dstLen);
    if (cread <= 0) {
        errnoPrint();
        exit(EXIT_FAILURE);
    }
    dst[cread - 1] = STRING_END;
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

void InitFdSet(FdSet* fdset, int UDPfd, int TCPfd) {
    fdset->max = 0;
    FD_ZERO(&fdset->master);
    FD_ZERO(&fdset->working);
    FD_SETTER(STDIN_FILENO, fdset);
    FD_SETTER(UDPfd, fdset);
    FD_SETTER(TCPfd, fdset);
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

int deserializer(char* msg, char** name, int* port, EXT* type) {
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

    if (rename(src, dst)) perror("Error moving log file");

    if (remove(src) == 0) perror("Failed to delete the file.");

    write(STDOUT_FILENO, "Exited successfully.\n", 21);
    exit(EXIT_SUCCESS);
}

int isUniqueName(char* name) {
    DIR* dir = opendir(LOG_FOLDER_ADD);
    if (dir == NULL) {
        perror("Unable to open directory");
        return -1;
    }

    int found = 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        char* namePart = strdup(entry->d_name);
        char* pos = strchr(namePart, '-');
        if (pos) *pos = '\0';  // Null-terminate the name at the first '-'.

        if (strcmp(namePart, name) == 0) {
            found = 1;
            free(namePart);
            break;
        }
        free(namePart);
    }

    closedir(dir);
    return !found;
}

void printWithType(int type) {
    DIR* dir = opendir(LOG_FOLDER_ADD);
    if (dir == NULL) {
        perror("Unable to open directory");
        return;
    }
    char name[20];
    int port, fileType;
    logLamination();
    logNormal("List of registered:");
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        sscanf(entry->d_name, "%[^-]-%d-%d.log", name, &port, &fileType);
        if (fileType == type && strcmp(name, ".") && strcmp(name, "over") && strcmp(name, "..")) {
            char out[BUF_CLI] = {STRING_END};
            sprintf(out, "Name: %s, Port: %d", name, port);
            logNormal(out);
        }
    }
    logLamination(); 

    closedir(dir);
}