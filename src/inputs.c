#include "../inputs.h"

#include <math.h>

inputs_mouse inputs_init_mouse() {
    inputs_mouse mouse = {0};
    mouse.ix = -1;
    return mouse;
}

#define BASE_ARGS filr_context *context, view_t *view

#define key_center_camera KEY_Z
#define key_move_one_down KEY_DOWN
#define key_move_one_up KEY_UP
#define key_file_action KEY_ENTER
#define key_mouse_left_click MOUSE_BUTTON_LEFT

#define HANDLE_INPUT(input, ...) if (IsKeyPressed(key_##input)) input(__VA_ARGS__)
#define HANDLE_INPUT_MOUSE(input, ...) if (IsMouseButtonPressed(key_##input)) input(__VA_ARGS__)
#define HANDLE_INPUT_SCROLL(...) mouse_scroll(__VA_ARGS__)


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
    filr_action(context);
    filr_reset_index(context);
    view_move_camera(context, view);
}

void mouse_left_click(BASE_ARGS, inputs_mouse *mouse) {
    if (context->file_index == mouse->ix) {
        filr_goto_directory(context);
        filr_reset_index(context);
        view_move_camera(context, view);
    } else if (mouse->ix != -1) context->file_index = mouse->ix;
}

void mouse_scroll(BASE_ARGS, inputs_mouse *mouse) {
    float mouse_wheel_move = 0;

    if ((mouse_wheel_move = GetMouseWheelMove())) {
        mouse->scroll_pos += (mouse_wheel_move > SCROLL_SPEED_CAP)? SCROLL_SPEED_CAP : mouse_wheel_move;
        if (fabs(mouse->scroll_pos) > SCROLL_STEP) {
            int step = (int) (mouse->scroll_pos / SCROLL_STEP);
            mouse->scroll_pos -= (float) (step) * SCROLL_STEP;
            filr_move_index(context, -step);
            view_center_camera(context, view);
        }
    }
}


void handle_key_presses(filr_context *context, view_t *view, inputs_mouse *mouse) {
    HANDLE_INPUT(center_camera, context, view);

    HANDLE_INPUT(move_one_down, context, view);
    
    HANDLE_INPUT(move_one_up, context, view);

    HANDLE_INPUT(file_action, context, view);

    HANDLE_INPUT_MOUSE(mouse_left_click, context, view, mouse);

    HANDLE_INPUT_SCROLL(context, view, mouse);
}
