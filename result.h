#ifndef RESULT_H
#define RESULT_H

#include <stdbool.h>

typedef struct {
    bool err;
    const char *message;
} result;

//TODO: better error handling
#define RESULT_ERR(m) (result){.err = true, .message = m}
#define RESULT_OK (result){.err = false, .message = NULL}

#endif
