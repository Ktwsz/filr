#include "../inputs.h"

#include <math.h>
#include <stdlib.h>

void inputs_init_mouse(inputs_mouse *mouse) {
    mouse->ix = -1;
}

#define BASE_ARGS filr_context *context, view_t *view
#define MOUSE_ARGS filr_context *context, view_t *view, inputs_mouse *mouse
#define ALL_ARGS filr_context *context, view_t *view, inputs_mouse *mouse, filr_cmp_array *cmp_array

#define key_center_camera KEY_Z
#define key_move_one_down KEY_DOWN
#define key_move_one_up KEY_UP
#define key_file_action KEY_ENTER
#define key_mouse_left_click MOUSE_BUTTON_LEFT
#define key_key_scroll_down KEY_DOWN
#define key_key_scroll_up KEY_UP
#define key_change_file_sorting KEY_S

#define HANDLE_INPUT(input, ...) if (IsKeyPressed(key_##input)) input(__VA_ARGS__)
#define HANDLE_INPUT_MOUSE(input, ...) if (IsMouseButtonPressed(key_##input)) input(__VA_ARGS__)
#define HANDLE_INPUT_MOUSE_SCROLL(...) mouse_scroll(__VA_ARGS__)


void center_camera(BASE_ARGS) {
    view_center_camera(context, view);
}

void move_one_down(BASE_ARGS) {
    filr_move_index(context, 1);
    view_move_camera(context, view);
}

void move_one_up(BASE_ARGS) {
    filr_move_index(context, -1);
    view_move_camera(context, view);
}

void file_action(BASE_ARGS) {
    bool dir_change = filr_action(context);
    if (dir_change) {
        filr_reset_index(context);
        view_move_camera(context, view);
    }
}

void mouse_left_click(MOUSE_ARGS) {
    if (context->file_index == mouse->ix) {
        file_action(context, view);
    } else if (mouse->ix != -1) context->file_index = mouse->ix;
}


void scroll(MOUSE_ARGS, float scroll_move) {
    mouse->scroll_pos += (scroll_move > SCROLL_SPEED_CAP)? SCROLL_SPEED_CAP : scroll_move;
    if (fabs(mouse->scroll_pos) > SCROLL_STEP) {
        int step = (int) (mouse->scroll_pos / SCROLL_STEP);
        mouse->scroll_pos -= (float) (step) * SCROLL_STEP;
        filr_move_index(context, -step);
        view_center_camera(context, view);
    }
}



void mouse_scroll(MOUSE_ARGS) {
    float mouse_wheel_move = 0;

    if ((mouse_wheel_move = GetMouseWheelMove())) {
        scroll(context, view, mouse, mouse_wheel_move);
    }
}

void key_scroll_down(MOUSE_ARGS) {
    scroll(context, view, mouse, SCROLL_SPEED_CAP);
}

void key_scroll_up(MOUSE_ARGS) {
    scroll(context, view, mouse, -SCROLL_SPEED_CAP);
}

void change_file_sorting(filr_context *context, filr_cmp_array *cmp_array) {
    cmp_array->ix = (cmp_array->ix + 1) % cmp_array->size;
    size_t ix = cmp_array->ix;

    qsort(&(context->files[2]), context->size - 2, sizeof(filr_file), cmp_array->array[ix]);
}

void handle_key_presses(ALL_ARGS) {
    HANDLE_INPUT(center_camera, context, view);

    HANDLE_INPUT(move_one_down, context, view);
    
    HANDLE_INPUT(move_one_up, context, view);

    HANDLE_INPUT(file_action, context, view);

    HANDLE_INPUT(change_file_sorting, context, cmp_array);

    HANDLE_INPUT_MOUSE(mouse_left_click, context, view, mouse);

    HANDLE_INPUT_MOUSE_SCROLL(context, view, mouse);
    //TODO: add scroll with key up/key down
}
