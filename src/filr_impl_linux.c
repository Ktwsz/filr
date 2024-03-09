#include "filr.h"
#include <dirent.h>

cstr CSTR_DASH = { .str = "/", .size = 1 };

void filr_parse_file(filr_file *dst, struct dirent *dir) { 

}
result filr_load_directory(filr_context *context) {
    return RESULT_OK;
    char *dir_name = context->directory.str;
    DIR *d = opendir(dir_name);
    if (!d) {
        return RESULT_ERR("ERR: filr_load_directory invalid directory");
    }

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        filr_file next_file;

        cstr dir_path;
        //cstr_concat()
        filr_parse_file(&next_file, dir);

        result err = filr_file_array_append(&context->files_all, &next_file);
        if (err.err)
            return err;
    }
    closedir(d);
    return RESULT_OK;
}

result filr_action(filr_context *context) {
    return RESULT_OK;
}

bool directory_exists(cstr dir) {
    return true;
}
