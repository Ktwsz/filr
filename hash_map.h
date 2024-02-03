#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdlib.h>
#include "result.h"
#include "cstr.h"

#define HASH_MAP_SIZE 500


typedef struct {
    cstr key;
    void *value;
} node;

typedef struct {
    node *values;
    size_t value_size;
    size_t capacity;
} hash_map;

result hash_map_init(hash_map *m, size_t value_size);

result hash_map_insert(hash_map *m, const char *key, void *value);

void *hash_map_get(hash_map *m, cstr key);

void hash_map_free(hash_map *m);

#endif
