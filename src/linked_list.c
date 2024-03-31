#include "linked_list.h"

#include <stdlib.h>

result list_init(list_t *guard) {
    guard = malloc(sizeof(list_t));
    if (guard == NULL) 
        return RESULT_ERR("ERR: list_init failed malloc");

    guard->head = -1;
    guard->tail = NULL;

    return RESULT_OK;
}

result list_query(list_t *list, int val) {
    list_t *it = list, *prev = NULL; 

    while (it->tail != NULL) {
        if (it->head == val) {
            list_t *remove = it;
            prev->tail = it->tail;

            free(remove);

            return RESULT_OK;
        }
        
        prev = it;
        it = it->tail;
    }

    result err = list_init(it->tail);
    if (err.err)
        return err;

    it->tail->head = val;

    return RESULT_OK;
}

void list_clear(list_t *list) {
    if (list == NULL)
        return;

    list_clear(list->tail);
    free(list);
}

int list_next(list_t *list) {
    int val = list->head;
    list = list->tail;

    return val;
}

bool list_end(list_t *list) {
    return list == NULL;
}
