#include "../inputs.h"

#include <math.h>
#include <stdlib.h>

void inputs_init(inputs_t *input) {
    input->mouse_ix = -1;
    input->scroll_pos = 0;
    input->mode = INPUTS_NORMAL;    
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

#define HANDLE_INPUT(input, ...) if (IsKeyPressed(key_##input)) input(__VA_ARGS__)
#define HANDLE_INPUT_MOUSE(input, ...) if (IsMouseButtonPressed(key_##input)) input(__VA_ARGS__)
#define HANDLE_INPUT_MOUSE_SCROLL(...) mouse_scroll(__VA_ARGS__)


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
    scroll(context, view, input, SCROLL_SPEED_CAP);
}

void key_scroll_up(INPUTS_ARGS) {
    scroll(context, view, input, -SCROLL_SPEED_CAP);
}

void change_file_sorting(filr_context *context, filr_cmp_array *cmp_array) {
    cmp_array->ix = (cmp_array->ix + 1) % cmp_array->size;
    size_t ix = cmp_array->ix;

    qsort(&(context->files[2]), context->size - 2, sizeof(filr_file), cmp_array->array[ix]);
}

void view_dotfiles(BASE_ARGS) {
    view->window.hide_dotfiles = !view->window.hide_dotfiles;
    if (view->window.hide_dotfiles && context->files[context->file_index].is_dotfile)
        move_one_down(context, view);
}

void handle_key_presses(ALL_ARGS) {
    if (input->mode == INPUTS_NORMAL) {
        HANDLE_INPUT(center_camera, context, view);

        HANDLE_INPUT(move_one_down, context, view);
        
        HANDLE_INPUT(move_one_up, context, view);

        HANDLE_INPUT(file_action, context, view);

        HANDLE_INPUT(change_file_sorting, context, cmp_array);

        HANDLE_INPUT(view_dotfiles, context, view); 

        HANDLE_INPUT_MOUSE(mouse_left_click, context, view, input);

        HANDLE_INPUT_MOUSE_SCROLL(context, view, input);
    }
    //TODO: add scroll with key up/key down
}
