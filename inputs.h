#ifndef INPUTS_H
#define INPUTS_H

#include "lib/raylib.h"
#include "view.h"
#include "filr.h"

typedef enum {
    INPUTS_NORMAL,
    INPUTS_CREATE_FILE,
    INPUTS_CREATE_DIRECTORY,
    INPUTS_RENAME
} mode_enum;

typedef struct {
    int mouse_ix;
    float scroll_pos;
    mode_enum mode;
    int scroll_frames_count;
    cstr input_str;
    bool second_window_open;
    int window_focus;
} inputs_t;


void inputs_init(inputs_t *input, view_t *view);

void handle_key_presses(filr_context *context, view_t *view, inputs_t *input);

void mouse_input_callback(const void *inputs_ptr, Rectangle rect, int ix);

#endif
