#ifndef INPUTS_H
#define INPUTS_H

#include "view.h"
#include "filr.h"

#define SCROLL_SPEED_CAP 1.0
#define SCROLL_STEP 0.5

typedef struct {
    int ix;
    float scroll_pos;
} inputs_mouse;

typedef enum {
    NORMAL, 
    INPUT
} mode;

void inputs_init_mouse(inputs_mouse *mouse);

void handle_key_presses(filr_context *context, view_t *view, inputs_mouse *mouse);

#endif
