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
        m->values[i].next = NULL;
    }
    return RESULT_OK;
}

node *get_last(hash_map *m, size_t key) {
    node *p = &m->values[key];
    while (p->next != NULL) p = p->next;
    return p;
}

result new_value(hash_map *m, node *p, cstr key_cstr, void *value) {
    p->next = malloc(sizeof(node));
    if (p->next == NULL)
        return RESULT_ERR("ERR: hash_map_insert malloc failed for allocating node memory");

    p->next->value = malloc(m->value_size);
    if (p->next->value == NULL)
        return RESULT_ERR("ERR: hash_map_insert malloc failed for allocating value memory");

    memcpy(p->next->value, value, m->value_size);
    cstr_copy(&p->next->key, key_cstr);
    p->next->next = NULL;

    return RESULT_OK;
}

result hash_map_insert(hash_map *m, const char *key, void *value) {
    cstr key_cstr;
    cstr_init_name(&key_cstr, key);

    size_t key_hash = cstr_hash(key_cstr) % m->capacity;

    node *p = get_last(m, key_hash);
    result err = new_value(m, p, key_cstr, value);
    return err;
}

void *hash_map_get(hash_map *m, cstr key) {
    size_t key_hash = cstr_hash(key) % m->capacity;

    node *p = m->values[key_hash].next;
    while (p != NULL) {
        cstr map_key = p->key;
        void *map_value = p->value;
        if (cstr_cmp(&map_key, &key) == 0)
            return map_value;

        p = p->next;
    }

    return NULL;
}

void free_node(node *p, bool is_first) {
    if (p->next != NULL) {
        free_node(p->next, false);
        free(p->next);
    }

    if (!is_first)
        free(p->value);
}

void hash_map_free(hash_map *m) {
    for (size_t i = 0; i < m->capacity; ++i) {
        if (m->values[i].value != NULL)
            free_node(m->values[i].next, true);
    }

    free(m->values);
}
