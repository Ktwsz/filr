#include "../filr.h"
#include <string.h>
#include <assert.h>

#include <windows.h>

#define assert_return(expr) if (!(expr)) return

cstr CSTR_DASH = { .str = "/", .size = 1 };

result filr_file_array_append(filr_context* context, filr_file* new_elem) {
    context->size++;
    if (!new_elem->is_dotfile) context->no_dotfiles_size++;

    if (context->files == NULL) {
        void *ptr = malloc(context->capacity * sizeof(filr_file));
        if (ptr == NULL)
            return RESULT_ERR("ERR: filr_file_array_append malloc failed");

        context->files = ptr;
    }

    if (context->size > context->capacity) {
        context->capacity *= 2;
        void *ptr= realloc(context->files, context->capacity * sizeof(filr_file));
        if (ptr == NULL)
            return RESULT_ERR("ERR: filr_file_array_append realloc failed");
    }

    memcpy(context->files + (context->size - 1), new_elem, sizeof(filr_file));
    return RESULT_OK;
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

    dst->is_dotfile = strcmp(dst->name.str, "..") != 0 && strcmp(dst->name.str, ".") != 0 && dst->name.str[0] == '.';
}


result filr_load_directory(filr_context *context) {
    WIN32_FIND_DATA file;
    HANDLE hFind = NULL;
    char *dir = context->directory.str;

    char str_path[2048];
    sprintf(str_path, "%s\\*.*", dir);

    if ((hFind = FindFirstFile(str_path, &file)) == INVALID_HANDLE_VALUE) {
        return RESULT_ERR("ERR: filr_load_directory invalid handle");
    }

    do {
        filr_file next_file;

        filr_parse_file(&next_file, file);

        result err = filr_file_array_append(context, &next_file);
        if (err.err)
            return err;
    } while (FindNextFile(hFind, &file));

    FindClose(hFind);

    return RESULT_OK;
}

result filr_init_context(filr_context *context) {
    context->capacity = INIT_ARRAY_CAPACITY;
    char *HOME = getenv("HOMEPATH");
    printf("%s\n", HOME);
    cstr_init_name(&(context->directory), HOME);
    
    return filr_load_directory(context);
}


void filr_init_cmp_array(filr_cmp_array *array) {
    array->size = 6;
    array->ix = 0;

    array->array[0] = filr_file_comparator_basic;
    array->array[1] = filr_file_comparator_size;
    array->array[2] = filr_file_comparator_size_inv;
    array->array[3] = filr_file_comparator_edit_date;
    array->array[4] = filr_file_comparator_extension;
    array->array[5] = filr_file_comparator_alphabetic;
}

void filr_free_context(filr_context *context) {
    free(context->files);
}

result filr_create_file(filr_context  *context, cstr file_name) {
    cstr file_path;
    cstr_init(&file_path, 0);
    cstr_concat(&file_path, 3, context->directory, CSTR_DASH, file_name);
    HANDLE handle = CreateFile(file_path.str,
                          GENERIC_WRITE,
                          0,
                          0,
                          CREATE_NEW,
                          FILE_ATTRIBUTE_NORMAL,
                          0);

    if (handle == INVALID_HANDLE_VALUE)
        return RESULT_ERR("ERR: filr_create_file error");

    CloseHandle(handle);
    return RESULT_OK;
}

result filr_rename_file(filr_context  *context, cstr new_file_name) {
    if (context->file_index <= 1)
        return RESULT_ERR("WARN: filr_rename_file file index less than 2");

    cstr old_file_name = *filr_get_name(context, context->file_index);
    cstr old_file_path;
    cstr_init(&old_file_path, 0);
    cstr_concat(&old_file_path, 3, context->directory, CSTR_DASH, old_file_name);

    cstr new_file_path;
    cstr_init(&new_file_path, 0);
    cstr_concat(&new_file_path, 3, context->directory, CSTR_DASH, new_file_name);

    bool err = MoveFile(old_file_path.str,
                        new_file_path.str);

    return (err) ? RESULT_OK : RESULT_ERR("ERR: filr_rename_file failed renaming");
}

result filr_delete_file(filr_context *context) {
    if (context->file_index <= 1)
        return RESULT_ERR("WARN: filr_delete_file file index less than 2");

    cstr file = *filr_get_name(context, context->file_index);
    cstr file_path;
    cstr_init(&file_path, 0);
    cstr_concat(&file_path, 3, context->directory, CSTR_DASH, file);

    bool err = DeleteFile(file_path.str);

    return (err) ? RESULT_OK : RESULT_ERR("ERR: filr_delete_file failed delete");
}

size_t filr_find_next_index(filr_context *context, int range, int step) {
    size_t ix = context->file_index + step;
    for (int ctr = 0; ix >= 0 && ix < context->size && ctr < range; ctr++) {
        while (ix >= 0 && ix < context->size && context->files[ix].is_dotfile) ix += step;
    }

    return ix;
}

void filr_reset(filr_context *context) {
    context->size = 0;
}

void filr_move_index(filr_context *context, int di, bool skip_dotfiles) {
    int new_index = (skip_dotfiles)? filr_find_next_index(context, (di > 0)? di : -di, (di > 0)? 1 : -1) : context->file_index + di;
    
    if (new_index < 0) {
        context->file_index = 0;
    } else if (new_index >= context->size) {
        context->file_index = context->size - 1;
    } else {
        context->file_index = new_index;
    }
}

void filr_move_index_filename(filr_context  *context, cstr filename) {
    for (size_t i = 0; i < context->size; ++i) {
        if (cstr_cmp(filr_get_name(context, i), &filename) == 0) {
            context->file_index = i;
            return;
        }
    }
}

void filr_reset_index(filr_context *context) {
    context->file_index = 0;
}


result filr_action(filr_context *context) {
    size_t ix = context->file_index;
    filr_file file = context->files[ix];

    if (file.is_directory) {
        return filr_goto_directory(context);
    }

    INT_PTR err = (INT_PTR) ShellExecute(NULL,
                 NULL,
                 file.name.str,
                 NULL,
                 context->directory.str,
                 0);

    return (err > 32) ? RESULT_OK : RESULT_ERR("ERR: filr_action failed call shell execute");
}

result filr_goto_directory(filr_context* context) {
    cstr goto_directory = *filr_get_name(context, context->file_index);

    if (strcmp(goto_directory.str, "..") == 0) {
        cstr tmp;
        cstr_strip_directory(&tmp, context->directory);
        
        cstr_copy(&(context->directory), tmp);
    } else if (strcmp(goto_directory.str, ".") != 0) {
        cstr tmp;
        cstr_concat(&tmp, 3, context->directory, CSTR_DASH, goto_directory);

        cstr_copy(&(context->directory), tmp);
    }

    filr_reset(context);
    return filr_load_directory(context);
}

cstr *filr_get_name(filr_context *context, size_t ix) {
    return &context->files[ix].name;
}


void filr_print_array(filr_context *context) {
    printf("size: %zu\n", context->size);
    for (size_t i = 0; i < context->size; ++i) {
        printf("i: %zu str size: %zu ", i, context->files[i].name.size);
        printf("%s\n", filr_get_name(context, i)->str);
    }

}

int filr_file_comparator_basic(const void *p1, const void *p2) {
    filr_file f1 = *((filr_file*)p1);
    filr_file f2 = *((filr_file*)p2);

    if (f1.is_directory == f2.is_directory)
        return cstr_cmp(&f1.name, &f2.name);

    if (f1.is_directory) return 1;
    return -1;
}

int filr_file_comparator_size(const void *p1, const void *p2) {
    filr_file f1 = *((filr_file*)p1);
    filr_file f2 = *((filr_file*)p2);
    
    return (f1.size == f2.size) ? 0 :
            (f1.size < f2.size) ? -1 : 1;
}

int filr_file_comparator_size_inv(const void *p1, const void *p2) {
    return -filr_file_comparator_size(p1, p2);

}

int filr_file_comparator_edit_date(const void *p1, const void *p2) {
    filr_file f1 = *((filr_file*)p1);
    filr_file f2 = *((filr_file*)p2);

    filr_date d1 = f1.last_edit_date;
    filr_date d2 = f2.last_edit_date;

    if (d1.year != d2.year)
        return d1.year - d2.year;
    if (d1.month != d2.month)
        return d1.month - d2.month;
    if  (d1.day != d2.day)
        return d1.day - d2.day;
    if (d1.hour != d2.hour)
        return d1.hour - d2.hour;

    return d1.minute - d2.minute;
}

int filr_file_comparator_extension(const void *p1, const void *p2) {
    filr_file f1 = *((filr_file*)p1);
    filr_file f2 = *((filr_file*)p2);

    cstr ext1, ext2;
    cstr_strip_extension(&ext1, f1.name);
    cstr_strip_extension(&ext2, f2.name);

    return cstr_cmp(&ext1, &ext2);
}

int filr_file_comparator_alphabetic(const void *p1, const void *p2) {
    filr_file f1 = *((filr_file*)p1);
    filr_file f2 = *((filr_file*)p2);
    
    return cstr_cmp(&f1.name, &f2.name);
}

size_t filr_count_dotfiles(filr_context *context, size_t ix) {
    int ctr = 0;
    for (size_t i = 0; i < ix; ++i) {
        if (context->files[i].is_dotfile) ctr++;
    }
    return ctr;
}
