#ifndef FILR_H
#define FILR_H


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "cstr.h"
#include "result.h"
#include "linked_list.h"

#define INIT_ARRAY_CAPACITY 128

typedef struct {
    unsigned short year, month, day, hour, minute;
} filr_date;

typedef struct {
    cstr name;
    cstr extension;
    bool is_directory;
    bool is_dotfile;
    size_t size;
    filr_date last_edit_date;
    size_t all_files_index;
} filr_file;

typedef struct {
    filr_file *files;
    size_t size;
    size_t capacity;
} filr_array;

typedef struct {
    size_t sorting_function_ix;
    bool hide_dotfiles;
} filr_view_config;

typedef int(*filr_comparator)(const void *, const void *);

typedef struct {
    filr_array files_all;
    filr_array files_visible;
    filr_view_config view_config;
    cstr directory;
    size_t visible_index;
    size_t no_dotfiles_size;
    filr_comparator cmp_array[6];

    list_t selected_files;
    filr_array selected_buffer;
} filr_context;


result filr_file_array_append(filr_array *array, filr_file *new_elem);

int filr_get_index_all(filr_context *context, int visible_ix);

result filr_load_directory(filr_context *context);

result filr_init_context(filr_context *context);

void filr_init_cmp_array(filr_comparator *array);

void filr_free_context(filr_context *context);

result filr_visible_update(filr_context *context);

result filr_next_sorting_ix(filr_context *context);

result filr_set_hide_dotfiles(filr_context *context, bool hide_dotfiles);

result filr_create_file(filr_context  *context, cstr file_name);

result filr_create_directory(filr_context  *context, cstr file_name);

result filr_rename_file(filr_context  *context, cstr file_name);

result filr_delete_file(filr_context *context);

result filr_copy_file(cstr old_file_path, cstr new_file_path);

result filr_move_file(cstr old_file_path, cstr new_file_path);

result filr_open_nvim(filr_context *context);

result filr_open_windows_explorer(filr_context *context);

void filr_move_index(filr_context *context, int di);

void filr_move_index_filename(filr_context *context, cstr filename);

result filr_action(filr_context *context);

result filr_goto_directory(filr_context *context);

cstr *filr_get_name_visible(filr_context *context, size_t ix);

cstr *filr_get_name_all(filr_context *context, size_t ix);

void filr_create_dummy_file(filr_file *dst);

cstr filr_setup_command(filr_context *context, const char *command_format);

result filr_select_toggle_file(filr_context *context);

bool filr_select_contains(filr_context *context, int ix);

void filr_select_clear(filr_context *context);

result filr_select_toggle_all(filr_context *context);

result filr_select_buffer_save(filr_context *context);

result filr_select_buffer_copy(filr_context *context, cstr new_directory);

result filr_select_buffer_move(filr_context *context, cstr new_directory);

void filr_select_buffer_clear(filr_context *context);

int filr_file_comparator_basic(const void *p1, const void *p2);

int filr_file_comparator_size(const void *p1, const void *p2);

int filr_file_comparator_size_inv(const void *p1, const void *p2);

int filr_file_comparator_edit_date(const void *p1, const void *p2);

int filr_file_comparator_extension(const void *p1, const void *p2);

int filr_file_comparator_alphabetic(const void *p1, const void *p2);

#endif
