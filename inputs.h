#ifndef INPUTS_H
#define INPUTS_H

#include "lib/raylib.h"
#include "view.h"
#include "filr.h"

#define SCROLL_SPEED_CAP 1.0
#define SCROLL_STEP 0.5
#define SCROLL_KEY_SPEED 0.25
#define SCROLL_FRAME_THRESHOLD 10

typedef enum {
    INPUTS_NORMAL,
    INPUTS_CREATE,
    INPUTS_RENAME,
    INPUTS_LOGGER
} mode_enum;

typedef struct {
    int mouse_ix;
    float scroll_pos;
    mode_enum mode;
    int scroll_frames_count;
    cstr input_str;
} inputs_t;


void inputs_init(inputs_t *input);

void handle_key_presses(filr_context *context, view_t *view, inputs_t *input);

void mouse_input_callback(const void *inputs_ptr, Rectangle rect, int ix);

#endif
