#include "utils.h"

void cliPrompt() { write(STDOUT_FILENO, ANSI_WHT ">> " ANSI_RST, 12); }

void errnoPrint() { logError(strerror(errno)); }

int writeToFile(const char* filename, const char* ext, const char* txt) {
    char fname[BUF_NAME + 10] = {'\0'};
    strcpy(fname, filename);
    if (ext != NULL) strcat(fname, ext);

    chmod(fname, S_IWUSR | S_IRUSR);
    int fd = open(fname, O_CREAT | O_WRONLY | O_APPEND);
    if (fd < 0) return 1;

    if (write(fd, txt, strlen(txt)) < 0) return 1;
    close(fd);
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

void loadFoodNames(Customer* customer) {
    cJSON* root = loadJSON();
    if (root == NULL) return;

    int foodSize = cJSON_GetArraySize(root);
    customer->foodSize = foodSize;

    cJSON* item = NULL;
    int index = 0;
    cJSON_ArrayForEach(item, root) {
        customer->foods[index] = strdup(item->string);
        index++;
    }

    cJSON_Delete(root);
}

loadMenu(Restaurant* restaurant) {
    cJSON* root = loadJSON();
    if (root == NULL) return;

    int menuSize = 0;
    cJSON* food_item = NULL;
    cJSON_ArrayForEach(food_item, root) {
        Food* food = &restaurant->menu[menuSize];
        strncpy(food->name, food_item->string, BUF_NAME);

        int ingredientSize = 0;
        cJSON* ingredient_item = NULL;
        cJSON_ArrayForEach(ingredient_item, food_item) {
            food->ingredients[ingredientSize] = strdup(ingredient_item->string);
            food->quantity[ingredientSize] = ingredient_item->valueint;
            ingredientSize++;
        }
        food->ingredientSize = ingredientSize;

        menuSize++;
    }
    restaurant->menuSize = menuSize;

    cJSON_Delete(root);
}