#include "linked_list.h"

#include <stdlib.h>

list_t list_init() {
    return (list_t){ .head = -1, .tail = NULL };
}

result list_new(list_t *guard, int val) {
    guard->tail = malloc(sizeof(list_t));
    if (guard->tail == NULL) 
        return RESULT_ERR("ERR: list_init failed malloc");

    guard->tail->head = val;
    guard->tail->tail = NULL;

    return RESULT_OK;
}

result list_query(list_t *list, int val) {
    list_t *it = list, *prev = NULL; 

    while (it != NULL) {
        if (it->head == val) {
            list_t *remove = it;
            prev->tail = it->tail;

            free(remove);

            return RESULT_OK;
        }
        
        prev = it;
        it = it->tail;
    }

    result err = list_new(prev, val);
    if (err.err)
        return err;

    return RESULT_OK;
}

void list_clear(list_t *list) {
    if (list == NULL)
        return;

    list_clear(list->tail);
    free(list);
}

bool list_contains(list_t *list, int val) {
    list_t *it = list; 

    while (it != NULL) {
        if (it->head == val)
            return true;

        it = it->tail;
    }

    return false;
}
