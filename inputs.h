#ifndef INPUTS_H
#define INPUTS_H

#include "view.h"
#include "filr.h"

#define SCROLL_SPEED_CAP 1.0
#define SCROLL_STEP 0.5

typedef enum {
    INPUTS_NORMAL, 
    INPUTS_INPUT
} mode_enum;

typedef struct {
    int mouse_ix;
    float scroll_pos;
    mode_enum mode;
} inputs_t;


void inputs_init(inputs_t *input);

void handle_key_presses(filr_context *context, view_t *view, inputs_t *input, filr_cmp_array *cmp_array);

#endif
