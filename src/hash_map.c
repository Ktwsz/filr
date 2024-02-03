#include <memory.h>
#include "../hash_map.h"

result hash_map_init(hash_map *m, size_t value_size) {
    m->capacity = HASH_MAP_SIZE;
    m->value_size = value_size;

    void *ptr = malloc(m->capacity * sizeof(node));
    if (ptr == NULL)
        return RESULT_ERR("ERR: hash_map_init malloc failed");

    m->values = ptr;
    for (size_t i = 0; i < m->capacity; ++i) {
        cstr_init(&m->values[i].key, 0);
        m->values[i].value = NULL;
    }
    return RESULT_OK;
}

result hash_map_insert(hash_map *m, const char *key, void *value) {
    void *ptr = malloc(m->value_size);
    if (ptr == NULL)
        return RESULT_ERR("ERR: hash_map_init malloc failed for allocating value memory");

    cstr key_cstr;
    cstr_init_name(&key_cstr, key);

    size_t key_hash = cstr_hash(key_cstr) % m->capacity;

    m->values[key_hash].value = ptr;
    memcpy(m->values[key_hash].value, value, m->value_size);
    cstr_copy(&m->values[key_hash].key, key_cstr);

    return RESULT_OK;
}

void *hash_map_get(hash_map *m, cstr key) { //TODO: add case for multiple values with the same hash in the future
    size_t key_hash = cstr_hash(key) % m->capacity;
    cstr map_key = m->values[key_hash].key;
    void *map_value = m->values[key_hash].value;

    return (cstr_cmp(&map_key, &key) == 0) ? map_value : NULL;
}

void hash_map_free(hash_map *m) {
    for (size_t i = 0; i < m->capacity; ++i) {
        if (m->values[i].value != NULL)
            free(m->values[i].value);
    }

    free(m->values);
}
