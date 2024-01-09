#include "filr.h"
#include <string.h>
#include <stdarg.h>
#include "windows.h"

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
    dst->name = strdup(src.cFileName);

    dst->is_directory = src.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;

    dst->size = src.nFileSizeHigh * (MAXDWORD + 1) + src.nFileSizeLow;
    filr_parse_date(&(dst->last_edit_date), src.ftLastWriteTime);
}

char* concat(int count, ...) {
    va_list ap;
    int i;

    // Find required length to store merged string
    int len = 1; // room for NULL
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
        len += strlen(va_arg(ap, char*));
    va_end(ap);

    // Allocate memory to concat strings
    char *merged = calloc(sizeof(char),len);
    int null_pos = 0;

    // Actually concatenate strings
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
    {
        char *s = va_arg(ap, char*);
        strcpy(merged+null_pos, s);
        null_pos += strlen(s);
    }
    va_end(ap);

    return merged;
}

bool load_directory(filr_context *context) {
    WIN32_FIND_DATA file;
    HANDLE hFind = NULL;
    char *dir = context->directory;
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
    context.directory = HOME;
    
    load_directory(&context);

    return context;
}


void filr_free_context(filr_context *context) {
    free(context->files);
    free(context->directory);
}


void filr_move_index(filr_context *context, size_t ix) {
    context->file_index = ix;
}

void filr_reset_index(filr_context *context) {
    context->file_index = 0;
}


void filr_goto_directory(filr_context* context) {
    size_t ix = context->file_index;
    char *goto_directory = context->files[ix].name;

    context->directory = concat(3, context->directory, "/", goto_directory);
    printf("%s\n", context->directory);

    load_directory(context);
}

/*int main(void) {

    const char *HOME = getenv("HOMEPATH");
    filr_file_array array = {0};
    array.capacity = INIT_ARRAY_CAPACITY;

    printf("%s\n", HOME);
    parse_directory_contents(HOME, &array);


    printf("------\n");

    for (size_t i = 0; i < array.size; ++i) {
        printf("%s %d %d\n", array.items[i].name, array.items[i].is_directory, array.items[i].size);
    }

    return 0;
}*/
