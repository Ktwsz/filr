#include "filr.h"
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <pwd.h>

cstr CSTR_DASH = { .str = "/", .size = 1 };

char *get_home_path() {
    struct passwd *pw = getpwuid(getuid());
    return pw->pw_dir;
}

void filr_parse_date(filr_date *dst, struct stat *file_stat) {
    char year[5], month[5], day[5], hour[5], minute[5];

    strftime(year, 5, "%Y", gmtime(&(file_stat->st_ctime)));
    strftime(month, 5, "%m", gmtime(&(file_stat->st_ctime)));
    strftime(day, 5, "%d", gmtime(&(file_stat->st_ctime)));
    strftime(hour, 5, "%H", gmtime(&(file_stat->st_ctime)));
    strftime(minute, 5, "%M", gmtime(&(file_stat->st_ctime)));

    dst->year = atoi(year);
    dst->month = atoi(month);
    dst->day = atoi(day);
    dst->hour = atoi(hour);
    dst->minute = atoi(minute);
}

void filr_init_dir(filr_file *dst, char *name) {
    cstr_init_name(&dst->name, name);
    cstr_init_name(&dst->extension, "folder");

    dst->is_dotfile = false;

    dst->is_directory = true;

    dst->last_edit_date = (filr_date){0};
    dst->size = 0;
}

result filr_parse_file(filr_file *dst, struct dirent *entry, cstr current_dir) { 
    cstr_init(&dst->extension, 0);
    cstr_init_name(&dst->name, entry->d_name);

    cstr entry_name, name_stat;
    cstr_init_name(&entry_name, entry->d_name);
    cstr_init(&name_stat, 0);
    cstr_concat(&name_stat, 3, current_dir, CSTR_DASH, entry_name);

    struct stat file_stat;
    if (stat(name_stat.str, &file_stat) == -1) {
        return RESULT_ERR("ERR filr_parse_file");
    }

    dst->is_directory = S_ISDIR(file_stat.st_mode);
    dst->size = file_stat.st_size;

    dst->is_dotfile = strcmp(dst->name.str, "..") != 0 && strcmp(dst->name.str, ".") != 0 && dst->name.str[0] == '.';

    if (dst->is_directory) {
        cstr_init_name(&dst->extension, "folder");
    } else if (dst->is_dotfile) {
        cstr_init_name(&dst->extension, "file");
    } else {
        cstr_strip_extension(&dst->extension, dst->name);
    }

    filr_parse_date(&dst->last_edit_date, &file_stat);
    return RESULT_OK;
}

bool is_dot_dir(struct dirent *entry) {
    return (strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".") == 0);
}

result filr_load_directory(filr_context *context) {
    filr_select_clear(context);

    char *dir_name = context->directory.str;
    DIR *dir = opendir(dir_name);
    if (dir == NULL) {
        return RESULT_ERR("ERR: filr_load_directory invalid directory");
    }

    result return_result = RESULT_OK;
    result err;

    filr_file dot, dot2;
    filr_init_dir(&dot, ".");
    filr_init_dir(&dot2, "..");

    err = filr_file_array_append(&context->files_all, &dot);
    if (err.err) {
        return_result = err;
        goto defer;
    }

    err = filr_file_array_append(&context->files_all, &dot2);
    if (err.err) {
        return_result = err;
        goto defer;
    }


    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (is_dot_dir(entry))
            continue;

        filr_file next_file;

        result file_err = filr_parse_file(&next_file, entry, context->directory);
        if (file_err.err) {
            return_result = file_err;
            goto defer;
        }

        err = filr_file_array_append(&context->files_all, &next_file);
        if (err.err) {
            return_result = err;
            goto defer;
        }
    }

    err = filr_visible_update(context);
    if (err.err) {
        return_result = err;
        goto defer;
    }

defer:
    closedir(dir);
    return return_result;
}

result filr_action(filr_context *context) {
    size_t ix = context->files_visible.files[context->visible_index].all_files_index;
    filr_file file = context->files_all.files[ix];

    if (file.is_directory) {
        return filr_goto_directory(context);
    }

    cstr abs_path;
    cstr_init(&abs_path, 0);
    cstr_concat(&abs_path, 3, context->directory, CSTR_DASH, file.name);

    cstr cmd_prefix;
    cstr_init_name(&cmd_prefix, "xdg-open ");

    cstr cmd;
    cstr_init(&cmd, 0);
    cstr_concat(&cmd, 2, cmd_prefix, abs_path);

    pid_t id = fork();
    if (id == -1)
        return RESULT_ERR("filr_action: couldnt create new proccess");

    if (id == 0) {
        system(cmd.str);
        exit(0);
    }

    return RESULT_OK;
}

bool directory_exists(cstr dir) {
    DIR *dir_p = opendir(dir.str);

    if (dir_p) {
        closedir(dir_p);
        return true;
    }
    return false;
}


result filr_open_nvim(filr_context *context) {
    cstr command = filr_setup_command(context, "cd %s && nvim .");

    pid_t id = fork();
    if (id == -1)
        return RESULT_ERR("filr_open_nvim: couldnt create new proccess");

    if (id == 0) {
        system(command.str);
        exit(0);
    }

    return RESULT_OK;
}


result filr_open_windows_explorer(filr_context *context) {
    cstr command = filr_setup_command(context, "xdg-open %s");

    pid_t id = fork();
    if (id == -1)
        return RESULT_ERR("filr_open_windows_explorer: couldnt create new proccess");

    if (id == 0) {
        system(command.str);
        exit(0);
    }

    return RESULT_OK;
}

//TODO
result filr_copy_file(cstr old_file_path, cstr new_file_path) {
    return RESULT_OK;
}

result filr_move_file(cstr old_file_path, cstr new_file_path) {
    int err = rename(old_file_path.str, new_file_path.str);

    if (err == -1)
        return RESULT_ERR("ERR: filr_move_file failed renaming");

    return RESULT_OK;
}
