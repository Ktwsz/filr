#include "filr.h"
#include <string.h>
#include "windows.h"

cstr CSTR_DASH = { .str = "/", .size = 1 };

void filr_file_array_append(filr_context* context, filr_file* new_elem) {
    context->size++;

    if (context->files == NULL) {
        context->files= malloc(context->capacity * sizeof(filr_file));
    }

    if (context->size > context->capacity) {
        context->capacity *= 2;
        void *_ = realloc(context->files, context->capacity * sizeof(filr_file));
    }

    memcpy(context->files+ (context->size - 1), new_elem, sizeof(filr_file));
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

void filr_parse_file(filr_file *dst, const WIN32_FIND_DATA src) { 
    char *tmp_str = strdup(src.cFileName);
    dst->name = cstr_init(strlen(tmp_str));
    dst->name.str = tmp_str;

    dst->is_directory = src.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;

    dst->size = src.nFileSizeHigh * (MAXDWORD + 1) + src.nFileSizeLow;
    filr_parse_date(&(dst->last_edit_date), src.ftLastWriteTime);
}


bool load_directory(filr_context *context) {
    WIN32_FIND_DATA file;
    HANDLE hFind = NULL;
    char *dir = context->directory.str;
    context->size = 0;

    char str_path[2048];
    sprintf(str_path, "%s\\*.*", dir);

    if ((hFind = FindFirstFile(str_path, &file)) == INVALID_HANDLE_VALUE) {
        return false;
    }

    do {
        filr_file next_file = {0};

        filr_parse_file(&next_file, file);

        filr_file_array_append(context, &next_file);
    } while (FindNextFile(hFind, &file));

    FindClose(hFind);

    return true;
}

filr_context filr_init_context() {
    filr_context context = {0};

    context.capacity = INIT_ARRAY_CAPACITY;
    char *HOME = getenv("HOMEPATH");
    context.directory = cstr_init_name(HOME);
    
    load_directory(&context);

    return context;
}


void filr_free_context(filr_context *context) {
    for (size_t i = 0; i < context->size; ++i) cstr_free(&context->files[i].name);
    free(context->files);
    cstr_free(&context->directory);
}


void filr_move_index(filr_context *context, size_t ix) {
    context->file_index = ix;
}

void filr_reset_index(filr_context *context) {
    context->file_index = 0;
}


void filr_goto_directory(filr_context* context) {
    size_t ix = context->file_index;
    cstr goto_directory = context->files[ix].name;

    if (strcmp(goto_directory.str, "..") == 0) {
        cstr tmp = cstr_strip_directory(context->directory);
        //cstr_free(&context->directory);
        context->directory = tmp;
    } else if (strcmp(goto_directory.str, ".") != 0) {
        cstr tmp = cstr_concat(3, context->directory, CSTR_DASH, goto_directory);
        //cstr_free(&context->directory);
        context->directory = tmp;
    }

    printf("%s %d\n", context->directory.str, context->directory.size);
    load_directory(context);
}


char *filr_get_file_name(filr_context *context, size_t ix) {
    return context->files[ix].name.str;
}
