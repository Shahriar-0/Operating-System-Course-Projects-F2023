#ifndef JSON_READER_H
#define JSON_READER_H

#include "define_values.h"

#define FILE_ADDR "recipes.json"
#define JSON_SIZE 4096

#define STAGE1 0
#define READING_FOOD 1
#define STAGE2 2
#define READING_SUP_NAME 3
#define STAGE3 4
#define READING_SUP_AMOUNT 5
#define STAGE4 6

struct Supply {
    char name[BUF_SIZE];
    int amount;
    struct Supply* next;
};

struct Food {
    char name[BUF_SIZE];
    struct Supply* supply_list;
    struct Food* next;
};

struct Food* generate_food_list();
struct Supply* generate_supply_list();
struct Food* initial_food();
struct Supply* initial_supply();
int is_repetitive(struct Supply* list, char* new_name);
void add_to_sup_list(struct Supply* head, struct Supply* new_sup);

#endif