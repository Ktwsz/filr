#include "../inputs.h"

#include <math.h>
#include <stdlib.h>

void inputs_init(inputs_t *input) {
    input->mouse_ix = -1;
    input->scroll_pos = 0;
    input->mode = INPUTS_NORMAL;    
    input->scroll_frames_count = 0;
    cstr_init(&input->input_str, 0);
}

#define BASE_ARGS filr_context *context, view_t *view
#define INPUTS_ARGS filr_context *context, view_t *view, inputs_t *input
#define ALL_ARGS filr_context *context, view_t *view, inputs_t *input, filr_cmp_array *cmp_array

#define key_center_camera KEY_Z
#define key_move_one_down KEY_DOWN
#define key_move_one_up KEY_UP
#define key_file_action KEY_ENTER
#define key_mouse_left_click MOUSE_BUTTON_LEFT
#define key_key_scroll_down KEY_DOWN
#define key_key_scroll_up KEY_UP
#define key_change_file_sorting KEY_S
#define key_view_dotfiles KEY_PERIOD
#define key_create_start KEY_T
#define key_create_confirm KEY_ENTER
#define key_delete_file KEY_D
#define key_rename_start KEY_R
#define key_rename_confirm KEY_ENTER
#define key_input_mode_cancel KEY_LEFT_SHIFT
#define key_input_mode_delete_last KEY_BACKSPACE

#define HANDLE_INPUT(input, ...) if (IsKeyPressed(key_##input)) input(__VA_ARGS__)
#define HANDLE_INPUT_MOUSE(input, ...) if (IsMouseButtonPressed(key_##input)) input(__VA_ARGS__)
#define HANDLE_INPUT_MOUSE_SCROLL(...) mouse_scroll(__VA_ARGS__)
#define HANDLE_INPUT_DOWN(input, ...) if (IsKeyDown(key_##input)) input(__VA_ARGS__)


void center_camera(BASE_ARGS) {
    view_center_camera(context, view);
}

void move_one_down(BASE_ARGS) {
    filr_move_index(context, 1, view->window.hide_dotfiles);
    view_move_camera(context, view);
}

void move_one_up(BASE_ARGS) {
    filr_move_index(context, -1, view->window.hide_dotfiles);
    view_move_camera(context, view);
}

void file_action(BASE_ARGS) {
    bool dir_change = filr_action(context);
    if (dir_change) {
        filr_reset_index(context);
        view_move_camera(context, view);
    }
}

void mouse_left_click(INPUTS_ARGS) {
    if (context->file_index == input->mouse_ix) {
        file_action(context, view);
    } else if (input->mouse_ix != -1) context->file_index = input->mouse_ix;
}


void scroll(INPUTS_ARGS, float scroll_move) {
    input->scroll_pos += (scroll_move > SCROLL_SPEED_CAP)? SCROLL_SPEED_CAP : scroll_move;
    if (fabs(input->scroll_pos) > SCROLL_STEP) {
        int step = (int) (input->scroll_pos / SCROLL_STEP);
        input->scroll_pos -= (float) (step) * SCROLL_STEP;
        filr_move_index(context, -step, view->window.hide_dotfiles);
        view_center_camera(context, view);
    }
}


void mouse_scroll(INPUTS_ARGS) {
    float mouse_wheel_move = 0;

    if ((mouse_wheel_move = GetMouseWheelMove())) {
        scroll(context, view, input, mouse_wheel_move);
    }
}

void key_scroll_down(INPUTS_ARGS) {
    if (input->scroll_frames_count < SCROLL_FRAME_THRESHOLD)
        input->scroll_frames_count++;
    else
        scroll(context, view, input, -SCROLL_KEY_SPEED);
}

void key_scroll_up(INPUTS_ARGS) {
    if (input->scroll_frames_count < SCROLL_FRAME_THRESHOLD)
        input->scroll_frames_count++;
    else
        scroll(context, view, input, SCROLL_KEY_SPEED);
}

void change_file_sorting(BASE_ARGS, filr_cmp_array *cmp_array) {
    cmp_array->ix = (cmp_array->ix + 1) % cmp_array->size;
    size_t ix = cmp_array->ix;

    qsort(&(context->files[2]), context->size - 2, sizeof(filr_file), cmp_array->array[ix]);
    view_center_camera(context, view);
}

void view_dotfiles(BASE_ARGS) {
    view->window.hide_dotfiles = !view->window.hide_dotfiles;
    if (view->window.hide_dotfiles && context->files[context->file_index].is_dotfile)
        move_one_down(context, view);
}

void input_mode_start(INPUTS_ARGS, mode_enum mode) {
    input->mode = mode;
    view_show_input(view);
}

void create_start(INPUTS_ARGS) {//TODO: add directory
    input_mode_start(context, view, input, INPUTS_CREATE);
}

void input_mode_cancel(INPUTS_ARGS) {
    input->mode = INPUTS_NORMAL;
    view_hide_input(view);
    cstr_init(&input->input_str, 0);
}

void create_confirm(INPUTS_ARGS) {
    filr_create_file(context, input->input_str);
    filr_reset(context);
    filr_load_directory(context);
    filr_move_index_filename(context, input->input_str);
    view_center_camera(context, view);

    input_mode_cancel(context, view, input);
}

void input_mode_delete_last(view_t *view, inputs_t *input) {
    cstr_pop(&input->input_str);
    cstr_pop(&view->input_str);
}

void delete_file(filr_context *context) {//TODO: delete directory
    filr_delete_file(context);
    filr_reset(context);
    filr_load_directory(context);
}


void input_mode_parse_key_queue(view_t *view, inputs_t *input) {
    int key_pressed = GetCharPressed();
    while(key_pressed) {
        if (key_pressed >= 32 && key_pressed <= 125)
            cstr_concat_single(&input->input_str, (char)key_pressed);

        key_pressed = GetCharPressed();
    }

    cstr_copy(&view->input_str, input->input_str);
}

void rename_start(INPUTS_ARGS) {
    input_mode_start(context, view, input, INPUTS_RENAME);

    cstr_copy(&view->input_str, *filr_get_name(context, context->file_index));
    cstr_copy(&input->input_str, *filr_get_name(context, context->file_index));
}

void rename_confirm(INPUTS_ARGS) {
    filr_rename_file(context, input->input_str);
    filr_reset(context);
    filr_load_directory(context);
    filr_move_index_filename(context, input->input_str);
    view_center_camera(context, view);

    input_mode_cancel(context, view, input);
}

void handle_key_presses(ALL_ARGS) {
    switch (input->mode) {
        case INPUTS_NORMAL:
            HANDLE_INPUT(center_camera, context, view);

            HANDLE_INPUT(move_one_down, context, view);

            HANDLE_INPUT(move_one_up, context, view);

            HANDLE_INPUT(file_action, context, view);

            HANDLE_INPUT(change_file_sorting, context, view, cmp_array);

            HANDLE_INPUT(view_dotfiles, context, view);

            HANDLE_INPUT(create_start, context, view, input);

            HANDLE_INPUT(delete_file, context);

            HANDLE_INPUT(rename_start, context, view, input);

            HANDLE_INPUT_MOUSE(mouse_left_click, context, view, input);

            HANDLE_INPUT_MOUSE_SCROLL(context, view, input);

            HANDLE_INPUT_DOWN(key_scroll_down, context, view, input);

            HANDLE_INPUT_DOWN(key_scroll_up, context, view, input);

            if (IsKeyUp(key_key_scroll_down) && IsKeyUp(key_key_scroll_up))
                input->scroll_frames_count = 0;
            break;

        case INPUTS_CREATE:
            HANDLE_INPUT(create_confirm, context, view, input);
            HANDLE_INPUT(input_mode_cancel, context, view, input);
            HANDLE_INPUT(input_mode_delete_last, view, input);
            input_mode_parse_key_queue(view, input);
            break;

        case INPUTS_RENAME:
            HANDLE_INPUT(rename_confirm, context, view, input);
            HANDLE_INPUT(input_mode_cancel, context, view, input);
            HANDLE_INPUT(input_mode_delete_last, view, input);
            input_mode_parse_key_queue(view, input);
            break;
    }
}

void mouse_input_callback(const void *inputs_ptr, Rectangle rect, int ix) {
    inputs_t *input = (inputs_t*)inputs_ptr;

    Vector2 mouse_position = GetMousePosition();

    if (CheckCollisionPointRec(mouse_position, rect))
        input->mouse_ix = ix;
}
