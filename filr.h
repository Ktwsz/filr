#ifndef FILR_H
#define FILR_H


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define INIT_ARRAY_CAPACITY 128

typedef struct {
    unsigned short year, month, day, hour, minute;
} filr_date;

typedef struct {
    const char* name;
    bool is_directory;
    size_t size;
    filr_date last_edit_date;
} filr_file;

typedef struct {
    filr_file *items;
    size_t size;
    size_t capacity;
} filr_file_array;


void filr_file_array_append(filr_file_array* array, filr_file* new_elem);

bool parse_directory_contents(const char *dir, filr_file_array* array);

filr_file_array filr_init_array();

filr_free_array(filr_file_array *array);

#endif
