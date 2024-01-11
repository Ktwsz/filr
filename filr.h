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
    bool hide_dotfiles;
    cstr directory;
    size_t file_index;
} filr_context;


void filr_file_array_append(filr_context* context, filr_file* new_elem);

bool load_directory(filr_context *context);

filr_context filr_init_context();

void filr_free_context(filr_context *context);

void filr_move_index(filr_context *context, size_t ix);

void filr_reset_index(filr_context *context);

void filr_goto_directory(filr_context *context);

char *filr_get_file_name(filr_context *context, size_t ix);

void filr_print_array(filr_context *context);

#endif
