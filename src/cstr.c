#include "../cstr.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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
