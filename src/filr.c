#include "../filr.h"
#include <string.h>
#include <assert.h>

#include <windows.h>

#define assert_return(expr) if (!(expr)) return

cstr CSTR_DASH = { .str = "\\", .size = 1 };
cstr CSTR_TRASH_DIR = { .str = "filr_trash", .size = 10 };

result filr_file_array_append(filr_array *array, filr_file* new_elem) {
    array->size++;

    if (array->files == NULL) {
        void *ptr = malloc(array->capacity * sizeof(filr_file));
        if (ptr == NULL)
            return RESULT_ERR("ERR: filr_file_array_append malloc failed");

        array->files = ptr;
    }

    if (array->size > array->capacity) {
        array->capacity *= 2;
        void *ptr= realloc(array->files, array->capacity * sizeof(filr_file));
        if (ptr == NULL)
            return RESULT_ERR("ERR: filr_file_array_append realloc failed");
    }

    memcpy(array->files + (array->size - 1), new_elem, sizeof(filr_file));
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
    cstr_init(&dst->extension, 0);

    dst->is_directory = src.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
    dst->is_dotfile = strcmp(dst->name.str, "..") != 0 && strcmp(dst->name.str, ".") != 0 && dst->name.str[0] == '.';

    if (dst->is_directory) {
        cstr_init_name(&dst->extension, "folder");
    } else if (dst->is_dotfile) {
        cstr_init_name(&dst->extension, "file");
    } else {
        cstr_strip_extension(&dst->extension, dst->name);
    }

    dst->size = src.nFileSizeHigh * (MAXDWORD + 1) + src.nFileSizeLow;
    filr_parse_date(&(dst->last_edit_date), src.ftLastWriteTime);
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

        result err = filr_file_array_append(&context->files_all, &next_file);
        if (err.err)
            return err;
    } while (FindNextFile(hFind, &file));

    FindClose(hFind);

    result err = filr_visible_update(context);
    if (err.err)
        return err;

    return RESULT_OK;
}


result filr_init_context(filr_context *context) {
    filr_init_cmp_array(context->cmp_array);

    context->files_all.capacity = INIT_ARRAY_CAPACITY;
    context->files_visible.capacity = INIT_ARRAY_CAPACITY;

    context->view_config.sorting_function_ix = 0;
    context->view_config.hide_dotfiles = true;

    char *HOME = getenv("HOMEPATH");
    cstr_init_name(&(context->directory), HOME);

    filr_create_directory(context, CSTR_TRASH_DIR);
    
    result load_err = filr_load_directory(context);
    if (load_err.err)
        return load_err;

    return RESULT_OK;
}


void filr_init_cmp_array(filr_comparator  *array) {
    array[0] = filr_file_comparator_basic;
    array[1] = filr_file_comparator_size;
    array[2] = filr_file_comparator_size_inv;
    array[3] = filr_file_comparator_edit_date;
    array[4] = filr_file_comparator_extension;
    array[5] = filr_file_comparator_alphabetic;
}

void filr_free_context(filr_context *context) {
    free(context->files_all.files);
    free(context->files_visible.files);
}

result filr_visible_update(filr_context *context) {
    context->files_visible.size = 0;
    for (size_t i = 0; i < context->files_all.size; ++i) {
        if (context->view_config.hide_dotfiles && context->files_all.files[i].is_dotfile)
            continue;

        result err = filr_file_array_append(&context->files_visible, &context->files_all.files[i]);
        if (err.err)
            return err;

        context->files_visible.files[context->files_visible.size - 1].all_files_index = i;
    }

    qsort(&(context->files_visible.files[2]), context->files_visible.size - 2, sizeof(filr_file), context->cmp_array[context->view_config.sorting_function_ix]);

    return RESULT_OK;
}

result filr_next_sorting_ix(filr_context *context) {
    size_t ix = context->view_config.sorting_function_ix;
    context->view_config.sorting_function_ix = (ix + 1) % 6;

    result err = filr_visible_update(context);
    if (err.err)
        return err;
    return RESULT_OK;
}

result filr_set_hide_dotfiles(filr_context *context, bool hide_dotfiles) {
    context->view_config.hide_dotfiles = hide_dotfiles;

    result err = filr_visible_update(context);
    if (err.err)
        return err;
    return RESULT_OK;
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
    if (context->visible_index <= 1)
        return RESULT_ERR("WARN: filr_rename_file file index less than 2");

    size_t file_index = context->files_visible.files[context->visible_index].all_files_index;

    cstr old_file_name = *filr_get_name_all(context, file_index);
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
    if (context->visible_index <= 1)
        return RESULT_ERR("WARN: filr_delete_file file index less than 2");

    size_t file_index = context->files_visible.files[context->visible_index].all_files_index;

    cstr file = *filr_get_name_all(context, file_index);
    cstr file_path;
    cstr_init(&file_path, 0);
    cstr_concat(&file_path, 3, context->directory, CSTR_DASH, file);

    char *HOME = getenv("HOMEPATH");
    cstr home;
    cstr_init_name(&home, HOME);

    cstr trash_file_path;
    cstr_concat(&trash_file_path, 5, home, CSTR_DASH, CSTR_TRASH_DIR, CSTR_DASH, file);

    bool err = MoveFile(file_path.str,
                        trash_file_path.str);

    return (err) ? RESULT_OK : RESULT_ERR("ERR: filr_delete_file failed delete");
}

bool directory_exists(cstr dir) {
    DWORD dwAttrib = GetFileAttributes(dir.str);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

result filr_create_directory(filr_context *context, cstr file_name) {
    cstr new_directory;
    cstr_concat(&new_directory, 3, context->directory, CSTR_DASH, file_name);

    if (directory_exists(new_directory))
        return RESULT_OK;

    bool err = CreateDirectory(new_directory.str, NULL);
    return (err) ? RESULT_OK : RESULT_ERR("ERR: filr_create_directory failed ");
}

result filr_open_windows_explorer(filr_context *context) {
    //TODO: thought this would be easier https://stackoverflow.com/questions/31563579/execute-command-using-win32
}

void filr_move_index(filr_context *context, int di) {
    int new_index = (int)context->visible_index + di;
    
    if (new_index < 0) {
        context->visible_index = 0;
    } else if (new_index >= context->files_visible.size) {
        context->visible_index = context->files_visible.size - 1;
    } else {
        context->visible_index = new_index;
    }
}

void filr_move_index_filename(filr_context  *context, cstr filename) {
    for (size_t i = 0; i < context->files_visible.size; ++i) {
        if (cstr_cmp(filr_get_name_visible(context, i), &filename) == 0) {
            context->visible_index = i;
            return;
        }
    }
    context->visible_index = 0;
}


result filr_action(filr_context *context) {
    size_t ix = context->files_visible.files[context->visible_index].all_files_index;
    filr_file file = context->files_all.files[ix];

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
    cstr goto_directory = *filr_get_name_visible(context, context->visible_index);

    cstr old_path;
    cstr_copy(&old_path, context->directory);

    bool backwards = false;
    cstr current_directory;

    if (strcmp(goto_directory.str, "..") == 0) {
        backwards = true;
        cstr_strip_suffix(&current_directory, context->directory, '\\');

        cstr tmp;
        cstr_strip_directory(&tmp, context->directory);
        
        cstr_copy(&(context->directory), tmp);
    } else if (strcmp(goto_directory.str, ".") != 0) {
        cstr tmp;
        cstr_concat(&tmp, 3, context->directory, CSTR_DASH, goto_directory);

        cstr_copy(&(context->directory), tmp);
    }

    context->files_all.size = 0;
    result err = filr_load_directory(context);
    if (err.err) {
        cstr_copy(&(context->directory), old_path);
        return err;
    }
    if (backwards)
        filr_move_index_filename(context, current_directory);
    else
        context->visible_index = 0;

    return RESULT_OK;
}


cstr *filr_get_name_visible(filr_context *context, size_t ix) {
    return &context->files_visible.files[ix].name;
}

cstr *filr_get_name_all(filr_context *context, size_t ix) {
    return &context->files_all.files[ix].name;
}

void filr_create_dummy_file(filr_file *dst) {
    cstr_init_name(&dst->name, "siala baba mak");
    dst->is_directory = false;
    dst->size = 6900;
    dst->last_edit_date.month = 4;
    dst->last_edit_date.day = 20;
    dst->last_edit_date.year = 2137;
    dst->last_edit_date.hour = 4;
    dst->last_edit_date.minute = 20;
}

int filr_file_comparator_basic(const void *p1, const void *p2) {
    filr_file f1 = *((filr_file*)p1);
    filr_file f2 = *((filr_file*)p2);

    if (f1.is_directory == f2.is_directory)
        return cstr_cmp_alphabetic(&f1.name, &f2.name);

    if (f1.is_directory) return -1;
    return 1;
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
