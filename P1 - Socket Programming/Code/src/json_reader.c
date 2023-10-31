#include "json_reader.h"

#include <fcntl.h>
#include <string.h>

struct Food* generate_food_list() {
    struct Food* head = initial_food();
    struct Food* food_iterator = head;
    struct Supply* sup_iterator = head->supply_list;

    int fd = open(FILE_ADDR, O_RDONLY);
    if (fd == ERROR) return;

    char buf[JSON_SIZE];
    int n = read(fd, buf, JSON_SIZE);
    if (n == ERROR) return;

    int state = STAGE1;
    int index;
    char integer_str[BUF_SIZE];
    for (int i = 0; 1; i++) {
        if (state == STAGE1 && buf[i] == '"') {
            struct Food* new_food = initial_food();
            food_iterator->next = new_food;
            food_iterator = new_food;
            sup_iterator = new_food->supply_list;
            state = READING_FOOD;
            index = 0;
        } else if (state == READING_FOOD && buf[i] != '"') {
            food_iterator->name[index] = buf[i];
            index++;
        } else if (state == READING_FOOD && buf[i] == '"') {
            food_iterator->name[index] = '\0';
            index = 0;
            state = STAGE2;
        } else if (state == STAGE2 && buf[i] == '"') {
            struct Supply* new_sup = initial_supply();
            sup_iterator->next = new_sup;
            sup_iterator = new_sup;
            state = READING_SUP_NAME;
            index = 0;
        } else if (state == READING_SUP_NAME && buf[i] != '"') {
            sup_iterator->name[index] = buf[i];
            index++;
        } else if (state == READING_SUP_NAME && buf[i] == '"') {
            sup_iterator->name[index] = '\0';
            index = 0;
            state = STAGE3;
        } else if (state == STAGE3 && buf[i] >= '1' && buf[i] <= '9') {
            memset(integer_str, '\0', BUF_SIZE);
            index = 0;
            integer_str[index] = buf[i];
            index++;
            state = READING_SUP_AMOUNT;
        } else if (state == READING_SUP_AMOUNT && buf[i] >= '0' && buf[i] <= '9') {
            integer_str[index] = buf[i];
            index++;
        } else if (state == READING_SUP_AMOUNT && (buf[i] < '0' || buf[i] > '9')) {
            integer_str[index] = '\0';
            index = 0;
            sup_iterator->amount = atoi(integer_str);
            state = STAGE4;
        } else if (state == STAGE4 && buf[i] == '"') {
            struct Supply* new_sup = initial_supply();
            sup_iterator->next = new_sup;
            sup_iterator = new_sup;
            state = READING_SUP_NAME;
            index = 0;
        } else if (state == STAGE4 && buf[i] == '}') {
            state = STAGE1;
        } else if (state == STAGE1 && buf[i] == '}') {
            break;
        }
    }

    return head;
}

struct Supply* generate_supply_list() {
    struct Supply* head = initial_supply();
    struct Supply* temp_sup;

    int fd = open(FILE_ADDR, O_RDONLY);
    if (fd == ERROR) return;

    char buf[JSON_SIZE];
    int n = read(fd, buf, JSON_SIZE);
    if (n == ERROR) return;

    int state = STAGE1;
    int index;
    for (int i = 0; 1; i++) {
        if (state == STAGE1 && buf[i] == '"') {
            state = READING_FOOD;
            index = 0;
        } else if (state == READING_FOOD && buf[i] == '"') {
            index = 0;
            state = STAGE2;
        } else if (state == STAGE2 && buf[i] == '"') {
            temp_sup = initial_supply();
            state = READING_SUP_NAME;
            index = 0;
        } else if (state == READING_SUP_NAME && buf[i] != '"') {
            temp_sup->name[index] = buf[i];
            index++;
        } else if (state == READING_SUP_NAME && buf[i] == '"') {
            temp_sup->name[index] = '\0';
            index = 0;
            if (is_repetitive(head, temp_sup->name))
                free(temp_sup);
            else
                add_to_sup_list(head, temp_sup);
            state = STAGE3;
        } else if (state == STAGE3 && buf[i] >= '1' && buf[i] <= '9') {
            state = READING_SUP_AMOUNT;
        } else if (state == READING_SUP_AMOUNT && (buf[i] < '0' || buf[i] > '9')) {
            state = STAGE4;
        } else if (state == STAGE4 && buf[i] == '"') {
            temp_sup = initial_supply();
            state = READING_SUP_NAME;
            index = 0;
        } else if (state == STAGE4 && buf[i] == '}') {
            state = STAGE1;
        } else if (state == STAGE1 && buf[i] == '}') {
            break;
        }
    }

    return head;
}

void add_to_sup_list(struct Supply* head, struct Supply* new_sup) {
    struct Supply* sup_iterator = head;

    while (sup_iterator->next != NULL) sup_iterator = sup_iterator->next;

    sup_iterator->next = new_sup;
}

int is_repetitive(struct Supply* list, char* new_name) {
    struct Supply* sup_iterator = list->next;

    while (sup_iterator != NULL) {
        if (strcmp(sup_iterator->name, new_name) == 0) return TRUE;
        sup_iterator = sup_iterator->next;
    }

    return FALSE;
}

struct Food* initial_food() {
    struct Food* new_food = (struct Food*)malloc(sizeof(struct Food));

    new_food->next = NULL;
    new_food->supply_list = initial_supply();
    memset(new_food->name, '\0', BUF_SIZE);

    return new_food;
}

struct Supply* initial_supply() {
    struct Supply* new_sup = (struct Supply*)malloc(sizeof(struct Supply));

    new_sup->next = NULL;
    new_sup->amount = 0;
    memset(new_sup->name, '\0', BUF_SIZE);

    return new_sup;
}
