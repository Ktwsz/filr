#include "filr.h"

void filr_file_array_append(filr_file_array* array, filr_file* new_elem) {
    array->size++;

    if (array->items == NULL) {
        array->items = malloc(array->capacity * sizeof(filr_file));
    }

    if (array->size > array->capacity) {
        array->capacity *= 2;
        realloc(array->items, array->capacity * sizeof(filr_file));
    }

    printf("%d ", array->size);
    memcpy(array->items + (array->size - 1), new_elem, sizeof(filr_file));
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

bool parse_directory_contents(const char *dir, filr_file_array* array) {
    WIN32_FIND_DATA file;
    HANDLE hFind = NULL;

    char str_path[2048];
    sprintf(str_path, "%s\\*.*", dir);

    if ((hFind = FindFirstFile(str_path, &file)) == INVALID_HANDLE_VALUE) {
        return false;
    }

    do {
        filr_file next_file = {0};

        filr_parse_file(&next_file, file);

        filr_file_array_append(array, &next_file);
    } while (FindNextFile(hFind, &file));

    return true;
}

int main(void) {

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
}
