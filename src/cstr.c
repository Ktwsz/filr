#include "cstr.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>

#define HASH_P 53
#define HASH_M 1000000009

#define assert_return(expr) if (!(expr)) return

void cstr_init(cstr *dst, size_t size) {
    dst->size = size;
    memset(dst->str, 0, MAX_STR_LEN);
}


void cstr_init_name(cstr *dst, const char *src) {
    snprintf(dst->str, MAX_STR_LEN, "%s", src);
    dst->size = strlen(src);
}

int cstr_cmp(const void *a, const void *b) {
    cstr val1 = *(cstr*)a;
    cstr val2 = *(cstr*)b;

    if (val1.size != val2.size) return (val1.size < val2.size) ? -1 : 1;
    return strcmp(val1.str, val2.str);
}

bool cstr_char_is_digit(char c) {
    return c >= '0' && c <= '9';
}

bool cstr_char_is_lowercase(char c) {
    return c >= 'a' && c <= 'z';
}

bool cstr_char_is_uppercase(char c) {
    return c >= 'A' && c <= 'Z';
}

int cstr_char_get_priority(char c) {
    if (cstr_char_is_digit(c)) return 0;
    if (cstr_char_is_lowercase(c)) return 1;
    if (cstr_char_is_uppercase(c)) return 2;
    return 3;
}

int cstr_char_cmp(char c1, char c2) {
    int p1 = cstr_char_get_priority(c1);
    int p2 = cstr_char_get_priority(c2);

    if (p1 != p2)
        return p1 - p2;

    return c1 - c2;
}

int cstr_cmp_alphabetic(const void *a, const void *b) {
    cstr val1 = *(cstr*)a;
    cstr val2 = *(cstr*)b;

    size_t n = (val1.size < val2.size)? val1.size : val2.size;
    for (size_t i = 0; i < n; ++i) {
        int val = cstr_char_cmp(val1.str[i], val2.str[i]);
        if (val != 0)
            return val;
    }

    return (int)(val1.size - val2.size);
}

void cstr_copy(cstr *dst, cstr src) {
    dst->size = src.size;
    memcpy(dst->str, src.str, (src.size + 1) * sizeof(char));
}


void cstr_concat(cstr*dst, int count, ...) {
    va_list ap;
    int i;

    // Find required length to store merged string
    size_t len = 0; // room for NULL
    va_start(ap, count);
    for(i = 0; i < count; i++) {
        cstr tmp = va_arg(ap, cstr);
        len += tmp.size;
    }
    va_end(ap);

    // Allocate memory to concat strings
    cstr_init(dst, len);
    size_t null_pos = 0;

    // Actually concatenate strings
    va_start(ap, count);
    for(i = 0; i < count; i++) {
        cstr s = va_arg(ap, cstr);
        strcpy(dst->str+null_pos, s.str);
        null_pos += s.size;
    }
    va_end(ap);
}

void cstr_concat_single(cstr *dst, const char c) {
    dst->size++;
    dst->str[dst->size-1] = c;
    dst->str[dst->size] = '\0';
}

void cstr_pop(cstr *dst) {
    assert_return(dst->size > 0);
    dst->size--;
    dst->str[dst->size] = '\0';
}

size_t cstr_find_last(cstr *dst, const char c) {
    size_t end_ix = dst->size-1;

    for (; end_ix > 0; --end_ix) {
        if (dst->str[end_ix] == c) return end_ix;
    }
    return 0;
}

void cstr_strip_suffix(cstr *dst, cstr src, const char c) {
    size_t end_ix = cstr_find_last(&src, c);

    cstr_init_name(dst, &src.str[end_ix + 1]);
}

void cstr_remove_suffix(cstr *dst, cstr src, const char c) {
    size_t end_ix = cstr_find_last(&src, c);

    cstr_init(dst, end_ix);
    memcpy(dst->str, src.str, end_ix);
}

void cstr_strip_directory(cstr *dst, cstr src, char sep) {
    cstr_remove_suffix(dst, src, sep);
}

void cstr_cap(cstr *dst, cstr src, int len) {
    cstr_init(dst, len);
    if (len >= src.size) {   
        sprintf(dst->str, "%s", src.str);
        char space = ' ';
        char *filler = calloc(sizeof(char), len - src.size + 1);
        memset(filler, space, len - src.size);

        sprintf(dst->str + src.size, "%s", filler);

        free(filler);
    } else {
        snprintf(dst->str, len - 2, "%s", src.str);
        sprintf(dst->str + len - 3, "...");
    }
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

void cstr_parse_file_size(cstr *dst, size_t file_size) {
    cstr_init(dst, 6);

    if (file_size == 0) {
        sprintf(dst->str, "%6s", "--");
        return;
    }

    int pow = 0;
    while(file_size >= 1000) {
        pow++;
        file_size /= 1000;
    }

    sprintf(dst->str, "%3zu %s", file_size, parse_size_prefix(pow));
}

void add_zero(char *s, int val) {
    if (val < 10) sprintf(s, "0%d", val);
    else sprintf(s, "%d", val);
}

void cstr_parse_date(cstr *dst, us day, us month, us year, us hour, us minute) {
    time_t t = time(NULL);
    struct tm tm = *gmtime(&t);
    int current_year = tm.tm_year + 1900;
    int current_month = tm.tm_mon;
    int current_day = tm.tm_mday;

    cstr_init(dst, 10);


    if (year == current_year && month == current_month && day == current_day) {
        char hour_str[3] = {0};
        add_zero(hour_str, hour);
        char minute_str[3] = {0};
        add_zero(minute_str, minute);

        sprintf(dst->str, "%7s:%s", hour_str, minute_str);
    } else {
        char day_str[3] = {0};
        add_zero(day_str, day);
        char month_str[3] = {0};
        add_zero(month_str, month);

        sprintf(dst->str, "%s.%s.%d", day_str, month_str, year);
    }
}

void cstr_strip_extension(cstr *dst, cstr src) {
    size_t i;
    for (i = src.size-1; i > 0; --i) {
        if (src.str[i] == '.') break;
    }

    cstr_init(dst, src.size - i - 1);

    sprintf(dst->str, "%s", src.str + i + 1);
}

void cstr_print(cstr s) {
    printf("%s\n", s.str);
}

size_t cstr_hash(cstr s) {
    const size_t p = HASH_P, m = HASH_M;
    size_t pow = 1;

    size_t result = 0;
    for (size_t i = 0; i < s.size; ++i){
        result = (result + (s.str[i]+ 1) * pow ) % m;
        pow *= p;
    }
    return result;
}

size_t cstr_hash_str(const char *s) {
    const size_t p = HASH_P, m = HASH_M;
    size_t pow = 1;

    size_t result = 0;
    while (*s) {
        result = (result + (*s++ + 1) * pow ) % m;
        pow *= p;
    }
    return result; 
}
