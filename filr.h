#ifndef FILR_H
#define FILR_H


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "cstr.h"

#define INIT_ARRAY_CAPACITY 128

typedef struct {
    unsigned short year, month, day, hour, minute;
} filr_date;

typedef struct {
    cstr name;
    bool is_directory;
    size_t size;
    filr_date last_edit_date;
} filr_file;

typedef struct {
    filr_file *files;
    size_t size;
    size_t capacity;
    cstr directory;
    size_t file_index;
} filr_context;

typedef int(*filr_comparator)(const void *, const void *);

typedef struct {
    filr_comparator array[6];
    size_t ix, size;
} filr_cmp_array;


void filr_file_array_append(filr_context* context, filr_file* new_elem);

bool load_directory(filr_context *context);

void filr_init_context(filr_context *context);

void filr_init_cmp_array(filr_cmp_array *array);

void filr_free_context(filr_context *context);

void filr_move_index(filr_context *context, int ix);

void filr_reset_index(filr_context *context);

bool filr_action(filr_context *context);

void filr_goto_directory(filr_context *context);

cstr filr_get_name_cstr(filr_context *context, size_t ix);

void filr_print_array(filr_context *context);

int filr_file_comparator_basic(const void *p1, const void *p2);

int filr_file_comparator_size(const void *p1, const void *p2);

int filr_file_comparator_size_inv(const void *p1, const void *p2);

int file_file_comparator_edit_date(const void *p1, const void *p2);

int file_file_comparator_extension(const void *p1, const void *p2);

int file_file_comparator_alphabetic(const void *p1, const void *p2);

#endif
