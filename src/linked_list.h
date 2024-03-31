#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "result.h"

typedef struct list_t {
    int head;
    struct list_t *tail;
} list_t;

result list_init(list_t *list);

result list_query(list_t *list, int val);

void list_clear(list_t *list);

int list_next(list_t *list);

bool list_end(list_t *list);

#endif
