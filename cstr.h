#ifndef CSTR_H
#define CSTR_H

#include <stddef.h>
#define MAX_STR_LEN 256

typedef struct {
    char str[MAX_STR_LEN];
    size_t size;
} cstr;

typedef unsigned short us;

cstr cstr_init(size_t size);
cstr cstr_init_name(const char *s);

void cstr_free(cstr *str);

cstr cstr_concat(int count, ...);

cstr cstr_strip_directory(cstr str);

cstr cstr_cap(cstr str, int len);

cstr cstr_parse_file_size(int file_size);

cstr cstr_parse_date(us day, us month, us year, us hour, us minute);

#endif
