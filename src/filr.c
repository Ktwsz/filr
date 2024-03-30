#include "filr.h"
#include <string.h>

#define assert_return(expr) if (!(expr)) return

cstr CSTR_TRASH_DIR = { .str = "filr_trash", .size = 10 };

#ifdef _WINDOWS_IMPL
#include "filr_impl_windows.c"
#else
#include <sys/types.h>
#include <fcntl.h>
#include "filr_impl_linux.c"
#endif

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

result filr_init_context(filr_context *context) {
    filr_init_cmp_array(context->cmp_array);

    context->files_all.capacity = INIT_ARRAY_CAPACITY;
    context->files_visible.capacity = INIT_ARRAY_CAPACITY;

    context->view_config.sorting_function_ix = 0;
    context->view_config.hide_dotfiles = true;

    char *HOME = get_home_path();

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

#ifdef _WINDOWS_IMPL
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
#else
    int err = open(file_path.str, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (err == -1)
        return RESULT_ERR("ERR: filr_create_file error");

    close(err);
#endif

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

#ifdef _WINDOWS_IMPL
    bool err = MoveFile(old_file_path.str,
                        new_file_path.str);

    if (!err)
        return RESULT_ERR("ERR: filr_rename_file failed renaming");
#else
    int err = rename(old_file_path.str, new_file_path.str);
    if (err == -1)
        return RESULT_ERR("ERR: filr_rename_file failed renaming");
#endif
    return RESULT_OK;
}

result filr_delete_file(filr_context *context) {
    if (context->visible_index <= 1)
        return RESULT_ERR("WARN: filr_delete_file file index less than 2");

    size_t file_index = context->files_visible.files[context->visible_index].all_files_index;

    cstr file = *filr_get_name_all(context, file_index);
    cstr file_path;
    cstr_init(&file_path, 0);
    cstr_concat(&file_path, 3, context->directory, CSTR_DASH, file);

    char *HOME = get_home_path();

    cstr home;
    cstr_init_name(&home, HOME);

    cstr trash_file_path;
    cstr_concat(&trash_file_path, 5, home, CSTR_DASH, CSTR_TRASH_DIR, CSTR_DASH, file);
    
#ifdef _WINDOWS_IMPL
    bool err = MoveFile(file_path.str,
                        trash_file_path.str);
    if (!err)
        return RESULT_ERR("ERR: filr_delete_file failed delete");
#else

    int err = rename(file_path.str, trash_file_path.str);
    if (err == -1)
        return RESULT_ERR("ERR: filr_delete_file failed delete");
#endif

    return RESULT_OK;
}

result filr_create_directory(filr_context *context, cstr file_name) {
    cstr new_directory;
    cstr_concat(&new_directory, 3, context->directory, CSTR_DASH, file_name);

    if (directory_exists(new_directory))
        return RESULT_OK;

#ifdef _WINDOWS_IMPL
    bool err = CreateDirectory(new_directory.str, NULL);
    if (!err)
        return RESULT_ERR("ERR: filr_create_directory failed ");
#else
    int err = mkdir(new_directory.str, 0777);
    if (err == -1)
        return RESULT_ERR("ERR: filr_create_directory failed");
#endif
    return RESULT_OK;
}

cstr filr_setup_command(filr_context *context, const char *command_format) {
    cstr full_path;
#ifdef _WINDOWS_IMPL
    cstr_concat(&full_path, 2, CSTR_C_DISC, context->directory);
#else
    cstr_init_name(&full_path, context->directory.str);
#endif

    cstr command;
    cstr_init(&command, 0);

    snprintf(command.str, MAX_STR_LEN, command_format, full_path.str);
    command.size = strlen(command.str);

    return command;
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

result filr_goto_directory(filr_context* context) {
    cstr goto_directory = *filr_get_name_visible(context, context->visible_index);

    cstr old_path;
    cstr_copy(&old_path, context->directory);

    bool backwards = false;
    cstr current_directory;

    if (strcmp(goto_directory.str, "..") == 0) {
        backwards = true;
        cstr_strip_suffix(&current_directory, context->directory, CSTR_DASH.str[0]);

        cstr tmp;
        cstr_strip_directory(&tmp, context->directory, CSTR_DASH.str[0]);
        
        if (tmp.size > 0)
            cstr_copy(&(context->directory), tmp);
        else
            cstr_init_name(&context->directory, "/");
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
