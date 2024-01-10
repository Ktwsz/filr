#ifndef CSTR_H
#define CSTR_H

#include <stddef.h>

typedef struct {
    char * str;
    size_t size;
} cstr;

cstr cstr_init(size_t size);
cstr cstr_init_name(char *s);

void cstr_free(cstr *str);

cstr cstr_concat(int count, ...);

cstr cstr_strip_directory(cstr str);

#endif
