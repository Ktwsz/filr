#include "filr.h"
#include <windows.h>

cstr CSTR_DASH = { .str = "\\", .size = 1 };
cstr CSTR_C_DISC = { .str = "C:", .size = 2 };

char *get_home_path() {
    return getenv("HOMEPATH");
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
    filr_select_clear(context);
    result return_result = RESULT_OK;

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
        if (err.err) {
            return_result = err;
            goto defer;
        }
    } while (FindNextFile(hFind, &file));

    result err = filr_visible_update(context);
    if (err.err) {
        return_result = err;
        goto defer;
    }

defer:
    FindClose(hFind);

    return return_result;
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

bool directory_exists(cstr dir) {
    DWORD dwAttrib = GetFileAttributes(dir.str);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}


result filr_open_nvim(filr_context *context) {
    cstr command = filr_setup_command(context, "start pwsh.exe -noexit -command \" cd %s && nvim .\"");

    system(command.str);
    return RESULT_OK;
}


result filr_open_windows_explorer(filr_context *context) {
    cstr command = filr_setup_command(context, "start %s");
    system(command.str);

    return RESULT_OK;
}
