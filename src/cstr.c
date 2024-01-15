#include "../cstr.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>


cstr cstr_init(size_t size) {
    cstr str = {0};
    str.size = size;

    return str;
}


cstr cstr_init_name(const char *s) {
    cstr str = {0};
    snprintf(str.str, MAX_STR_LEN, "%s", s);
    str.size = strlen(s);
    
    return str;
}


cstr cstr_concat(int count, ...) {
    va_list ap;
    int i;

    // Find required length to store merged string
    int len = 0; // room for NULL
    va_start(ap, count);
    for(i = 0; i < count; i++) {
        cstr tmp = va_arg(ap, cstr);
        len += tmp.size;
    }
    va_end(ap);

    // Allocate memory to concat strings
    cstr merged = cstr_init(len);
    int null_pos = 0;

    // Actually concatenate strings
    va_start(ap, count);
    for(i = 0; i < count; i++) {
        cstr s = va_arg(ap, cstr);
        strcpy(merged.str+null_pos, s.str);
        null_pos += s.size;
    }
    va_end(ap);

    return merged;
}

cstr cstr_strip_directory(cstr str) {
    size_t end_ix = str.size-1;
    
    for (; end_ix > 0; --end_ix) {
        if (str.str[end_ix] == '\\' || str.str[end_ix] == '/') break;
    }

    cstr result = cstr_init(end_ix);
    memcpy(result.str, str.str, end_ix);

    return result;
}

cstr cstr_cap(cstr str, int len) {
    cstr result = cstr_init(len);
    if (len >= str.size) {   
        sprintf(result.str, "%s", str.str);
        char space = ' ';
        char *filler = calloc(sizeof(char), len - str.size + 1);
        memset(filler, space, len - str.size);

        sprintf(result.str + str.size, "%s", filler);

        free(filler);
    } else {
        snprintf(result.str, len - 3, "%s", str.str);
        sprintf(result.str + len - 3, "...");
    }

   return result; 
}

const char *parse_size_prefix(int pow) {
    switch (pow) {
        case 0:
            return "B";
        case 1:
            return "KB";
        case 2:
            return "MB";
        case 3:
            return "GB";
        case 4:
            return "TB";
        default:
            return "OO";
    }
}

cstr cstr_parse_file_size(int file_size) {
    cstr result = cstr_init(6);

    if (file_size == 0) {
        sprintf(result.str, "%6s", "--");
        return result;
    }

    int pow = 0;
    while(file_size >= 1000) {
        pow++;
        file_size /= 1000;
    }

    sprintf(result.str, "%3d %s", file_size, parse_size_prefix(pow));
    return result;
}

void add_zero(char *s, int val) {
    if (val < 10) sprintf(s, "0%d", val);
    else sprintf(s, "%d", val);
}

cstr cstr_parse_date(us day, us month, us year, us hour, us minute) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    int current_year = tm.tm_year + 1900;
    int current_month = tm.tm_mon;
    int current_day = tm.tm_mday;

    cstr result = cstr_init(10);


    if (year == current_year && month == current_month && day == current_day) {
        char hour_str[3] = {0};
        add_zero(hour_str, hour);
        char minute_str[3] = {0};
        add_zero(minute_str, minute);

        sprintf(result.str, "%7s:%s", hour_str, minute_str);
    } else {
        char day_str[3] = {0};
        add_zero(day_str, day);
        char month_str[3] = {0};
        add_zero(month_str, month);

        sprintf(result.str, "%s.%s.%d", day_str, month_str, year);
    }

    return result;
}

cstr cstr_strip_extension(cstr s) {
    int i;
    for (i = s.size-1; i >= 0; --i) {
        if (s.str[i] == '.') break;
    }

    cstr result = cstr_init(s.size - i - 1);

    sprintf(result.str, "%s", s.str + i + 1);
    return result;
}
