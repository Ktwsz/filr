#include "filr.h"
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

cstr CSTR_DASH = { .str = "/", .size = 1 };

void filr_parse_date(filr_date *dst) {
    //TODO
    struct timespec ts;    
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    struct tm *my_tm = localtime(&ts.tv_sec);

    dst->year = my_tm->tm_year;
    dst->month = my_tm->tm_mon;
    dst->day = my_tm->tm_mday;
    dst->hour = my_tm->tm_hour;
    dst->minute = my_tm->tm_min;
}

result filr_parse_file(filr_file *dst, struct dirent *entry, cstr current_dir) { 
    if (strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".") == 0)
        return RESULT_OK;

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

    filr_parse_date(&dst->last_edit_date);
    return RESULT_OK;
}

result filr_load_directory(filr_context *context) {
    char *dir_name = context->directory.str;
    DIR *dir = opendir(dir_name);
    if (dir == NULL) {
        return RESULT_ERR("ERR: filr_load_directory invalid directory");
    }

    result return_result = RESULT_OK;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        filr_file next_file;

        result file_err = filr_parse_file(&next_file, entry, context->directory);
        if (file_err.err) {
            return_result = file_err;
            goto defer;
        }

        result err = filr_file_array_append(&context->files_all, &next_file);
        if (err.err) {
            return_result = err;
            goto defer;
        }
    }

defer:
    closedir(dir);
    return return_result;
}

result filr_action(filr_context *context) {
    return RESULT_OK;
}

bool directory_exists(cstr dir) {
    return true;
}
