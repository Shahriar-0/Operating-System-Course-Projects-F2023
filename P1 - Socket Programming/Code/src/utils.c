#include "utils.h"

////////////////////// Utils //////////////////////
void cliPrompt() { write(STDOUT_FILENO, ANSI_WHT ">> " ANSI_RST, 12); }

void errnoPrint() { logError(strerror(errno)); }

int writeToFile(const char* filename, const char* ext, const char* txt) {
    logInfo("Writing to file.");
    char fname[BUF_NAME + 10] = {'\0'};
    strcpy(fname, filename);
    if (ext != NULL) strcat(fname, ext);

    chmod(fname, S_IWUSR | S_IRUSR);
    int fd = open(fname, O_CREAT | O_WRONLY | O_APPEND);
    if (fd < 0) return 1;

    if (write(fd, txt, strlen(txt)) < 0) return 1;
    close(fd);
    logInfo("Done writing to file.");
    return 0;
}

void printNum(int fd, int num) {
    char buffer[12] = {'\0'};
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
    dst[cread - 1] = '\0';
}

int strToInt(const char* str, int* res) {
    char* end;
    long num = strtol(str, &end, 10);

    if (*end != '\0') return 1;
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
        logError("Port should be a number.");
        exit(EXIT_FAILURE);
    } else if (res == 2) {
        logError("Port number (16-bit) out of range.");
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

    logInfo("Socket added to fdset.");
}

void FD_CLRER(int socket, FdSet* fdset) {
    FD_CLR(socket, &fdset->master);

    if (socket == fdset->max) fdset->max--;

    logInfo("Socket removed from fdset.");
}

void InitFdSet(FdSet* fdset, int UDPfd) {
    logInfo("Initializing fdset.");
    fdset->max = 0;
    FD_ZERO(&fdset->master);
    FD_ZERO(&fdset->working);
    FD_SETTER(STDIN_FILENO, fdset);
    FD_SETTER(UDPfd, fdset);
    logInfo("Fdset initialized.");
}

////////////////////// JSON //////////////////////
char* read_file(const char* filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        logError("error in open file");
        return NULL;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        logError("Cannot get file size");
        close(fd);
        return NULL;
    }

    char* buffer = malloc(st.st_size + 1);
    if (buffer == NULL) {
        logError("Cannot allocate buffer");
        close(fd);
        return NULL;
    }

    ssize_t bytes_read = read(fd, buffer, st.st_size);
    if (bytes_read != st.st_size) {
        logError("Cannot read file");
        free(buffer);
        close(fd);
        return NULL;
    }

    buffer[st.st_size] = '\0';

    close(fd);
    return buffer;
}

cJSON* loadJSON() {
    char* jsonAddress = RECIPE_ADDRESS;
    char* json = read_file(jsonAddress);
    cJSON* root = cJSON_Parse(json);
    if (root == NULL) {
        char* error_ptr = cJSON_GetErrorPtr();
        char errmsg[BUF_MSG] = {'\0'};
        sprintf(errmsg, "Error before: %s\n", error_ptr);
        logError(errmsg);
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
        logInfo(msg);
        send(ansFd, ACCEPTED_MSG, strlen(ACCEPTED_MSG), 0);
    } else if (!strcmp(ans, "n")) {
        snprintf(msg, BUF_MSG, "rejected by %s.", name);
        logInfo(msg);
        send(ansFd, REJECTED_MSG, strlen(REJECTED_MSG), 0);
    } else {
        logError("Invalid answer.");
    }
}