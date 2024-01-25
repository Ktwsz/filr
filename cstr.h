#ifndef CSTR_H
#define CSTR_H

#include <stddef.h>
#define MAX_STR_LEN 256

typedef struct {
    char str[MAX_STR_LEN];
    size_t size;
} cstr;

typedef unsigned short us;

void cstr_init(cstr *dst, size_t size);

void cstr_init_name(cstr *dst, const char *src);

void cstr_copy(cstr *dst, cstr src);

void cstr_concat(cstr *dst, int count, ...);

void cstr_concat_single(cstr *dst, const char c);

void cstr_strip_directory(cstr *dst, cstr src);

void cstr_cap(cstr *dst, cstr src, int len);

void cstr_parse_file_size(cstr *dst, int file_size);

void cstr_parse_date(cstr *dst, us day, us month, us year, us hour, us minute);

void cstr_strip_extension(cstr *dst, cstr src);

int cstr_hash(char *s);

#endif
