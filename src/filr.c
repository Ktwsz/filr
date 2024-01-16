#include "../filr.h"
#include <string.h>
#include "windows.h"

cstr CSTR_DASH = { .str = "/", .size = 1 };

void filr_file_array_append(filr_context* context, filr_file* new_elem) {
    context->size++;

    if (context->files == NULL) {
        context->files = malloc(context->capacity * sizeof(filr_file));
    }

    if (context->size > context->capacity) {
        context->capacity *= 2;
        realloc(context->files, context->capacity * sizeof(filr_file));
    }

    memcpy(context->files + (context->size - 1), new_elem, sizeof(filr_file));
}

void filr_parse_date(filr_date *dst, const FILETIME src) {
    SYSTEMTIME sys_time = {0};

    if (!FileTimeToSystemTime(&src, &sys_time)) return;

    dst->year = sys_time.wYear;
    dst->month = sys_time.wMonth;
    dst->day = sys_time.wDay;
    dst->hour = sys_time.wHour;
    dst->minute = sys_time.wMinute;
}

void filr_parse_file(filr_file *dst, WIN32_FIND_DATA src) { 
    cstr_init_name(&(dst->name), src.cFileName);

    dst->is_directory = src.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;

    dst->size = src.nFileSizeHigh * (MAXDWORD + 1) + src.nFileSizeLow;
    filr_parse_date(&(dst->last_edit_date), src.ftLastWriteTime);
}


bool load_directory(filr_context *context) {
    WIN32_FIND_DATA file;
    HANDLE hFind = NULL;
    char *dir = context->directory.str;

    char str_path[2048];
    sprintf(str_path, "%s\\*.*", dir);

    if ((hFind = FindFirstFile(str_path, &file)) == INVALID_HANDLE_VALUE) {
        return false;
    }

    do {
        filr_file next_file;

        filr_parse_file(&next_file, file);

        filr_file_array_append(context, &next_file);
    } while (FindNextFile(hFind, &file));

    FindClose(hFind);

    return true;
}

void filr_init_context(filr_context *context) {
    context->capacity = INIT_ARRAY_CAPACITY;
    char *HOME = getenv("HOMEPATH");
    printf("%s\n", HOME);
    cstr_init_name(&(context->directory), HOME);
    
    load_directory(context);
}

void filr_free_context(filr_context *context) {
    free(context->files);
}


void filr_move_index(filr_context *context, int di) {
    int new_size = (int)context->file_index + di;

    if (new_size < 0) {
        context->file_index = 0;
    } else if (new_size >= context->size) {
        context->file_index = context->size - 1;
    } else {
        context->file_index = new_size;
    }
}

void filr_reset_index(filr_context *context) {
    context->file_index = 0;
}


bool filr_action(filr_context *context) {
    size_t ix = context->file_index;
    filr_file file = context->files[ix];

    if (file.is_directory) {
        filr_goto_directory(context);
        return true;
    }

    ShellExecute(NULL,
                 NULL,
                 file.name.str,
                 NULL,
                 context->directory.str,
                 0);

    return false;
}

void filr_goto_directory(filr_context* context) {
    size_t ix = context->file_index;
    cstr goto_directory = context->files[ix].name;

    if (strcmp(goto_directory.str, "..") == 0) {
        cstr tmp;
        cstr_strip_directory(&tmp, context->directory);
        
        cstr_copy(&(context->directory), tmp);
    } else if (strcmp(goto_directory.str, ".") != 0) {
        cstr tmp;
        cstr_concat(&tmp, 3, context->directory, CSTR_DASH, goto_directory);

        cstr_copy(&(context->directory), tmp);
    }

    context->size = 0;
    load_directory(context);
}

cstr filr_get_name_cstr(filr_context *context, size_t ix) {
    return context->files[ix].name;
}


void filr_print_array(filr_context *context) {
    printf("size: %d\n", context->size);
    for (size_t i = 0; i < context->size; ++i) {
        printf("i: %d str size: %d ", i, context->files[i].name.size);
        printf("%s\n", filr_get_name_cstr(context, i).str);
    }

}
