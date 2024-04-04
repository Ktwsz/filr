#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "result.h"

typedef struct list_t {
    int head;
    struct list_t *tail;
} list_t;

list_t list_init();

result list_new(list_t *list, int val);

result list_query(list_t *list, int val);

void list_clear(list_t *list);

bool list_contains(list_t *list, int val);

#endif
