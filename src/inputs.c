#include "../inputs.h"

#include <math.h>
#include <stdlib.h>


#define BASE_ARGS filr_context *context, view_t *view
#define INPUTS_ARGS filr_context *context, view_t *view, inputs_t *input
#define ALL_ARGS filr_context *context, view_t *view, inputs_t *input

#define key_center_camera KEY_Z
#define key_move_one_down KEY_DOWN
#define key_move_one_up KEY_UP
#define key_file_action KEY_ENTER
#define key_mouse_left_click MOUSE_BUTTON_LEFT
#define key_key_scroll_down KEY_DOWN
#define key_key_scroll_up KEY_UP
#define key_change_file_sorting KEY_S
#define key_view_dotfiles KEY_PERIOD
#define key_create_file_start KEY_T
#define key_create_file_confirm KEY_ENTER
#define key_create_dir_confirm KEY_ENTER
#define key_delete_file KEY_D
#define key_rename_start KEY_R
#define key_rename_confirm KEY_ENTER
#define key_input_mode_cancel KEY_C
#define key_input_mode_delete_last KEY_BACKSPACE
#define key_create_directory_start KEY_D

#define HANDLE_INPUT(input, ...) if (!IsKeyDown(KEY_LEFT_SHIFT) && !IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(key_##input)) input(__VA_ARGS__)
#define HANDLE_INPUT_SHIFT(input, ...) if (IsKeyDown(KEY_LEFT_SHIFT) && !IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(key_##input)) input(__VA_ARGS__)
#define HANDLE_INPUT_CTRL(input, ...) if (!IsKeyDown(KEY_LEFT_SHIFT) && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(key_##input)) input(__VA_ARGS__)
#define HANDLE_INPUT_MOUSE(input, ...) if (IsMouseButtonPressed(key_##input)) input(__VA_ARGS__)
#define HANDLE_INPUT_MOUSE_SCROLL(...) mouse_scroll(__VA_ARGS__)
#define HANDLE_INPUT_DOWN(input, ...) if (IsKeyDown(key_##input)) input(__VA_ARGS__)

cstr LOGGER_NORMAL_MODE_PREFIX = { .str = "NORMAL |", .size = 8 };
cstr LOGGER_CREATE_FILE_MODE_PREFIX = { .str = "FILE   |", .size = 8 };
cstr LOGGER_CREATE_DIR_MODE_PREFIX = { .str = "DIR    |", .size = 8 };
cstr LOGGER_RENAME_MODE_PREFIX = { .str = "RENAME |", .size = 8 };

void logger_mode_changed(view_t *view, inputs_t *input);

void inputs_init(inputs_t *input, view_t *view) {
    input->mouse_ix = -1;
    input->scroll_pos = 0;
    input->mode = INPUTS_NORMAL;
    input->scroll_frames_count = 0;
    cstr_init(&input->input_str, 0);

    logger_mode_changed(view, input);
}


void input_mode_cancel(INPUTS_ARGS);

void logger_setup_err(INPUTS_ARGS, result err) {
    view_logger_set_err(view, err);
    if (input->mode == INPUTS_RENAME || input->mode == INPUTS_CREATE_FILE || input->mode == INPUTS_CREATE_DIRECTORY)
        input_mode_cancel(context, view, input);
}

void logger_mode_changed(view_t *view, inputs_t *input) {
    view_logger_clear_err(view);
    switch (input->mode) {
        case INPUTS_NORMAL:
            view_window_set_str(&view->logger, LOGGER_NORMAL_MODE_PREFIX);
            break;
        case INPUTS_CREATE_FILE:
            view_window_set_str(&view->logger, LOGGER_CREATE_FILE_MODE_PREFIX);
            break;
        case INPUTS_CREATE_DIRECTORY:
            view_window_set_str(&view->logger, LOGGER_CREATE_DIR_MODE_PREFIX);
            break;
        case INPUTS_RENAME:
            view_window_set_str(&view->logger, LOGGER_RENAME_MODE_PREFIX);
            break;
    }
}

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

void file_action(INPUTS_ARGS) {
    bool dir_change = context->files_visible.files[context->visible_index].is_directory;
    result err = filr_action(context);
    if (err.err) {
        logger_setup_err(context, view, input, err);
        return;
    }

    if (dir_change) {
        context->visible_index = 0;
        view_move_camera(context, view);
    }
}

void mouse_left_click(INPUTS_ARGS) {
    if (context->visible_index == input->mouse_ix) {
        file_action(context, view, input);
    } else if (input->mouse_ix != -1) context->visible_index = input->mouse_ix;
}


//TODO: scroll jumps by 2 files
void scroll(INPUTS_ARGS, float scroll_move) {
    input->scroll_pos += (scroll_move > SCROLL_SPEED_CAP)? SCROLL_SPEED_CAP : scroll_move;
    if (fabs(input->scroll_pos) > SCROLL_STEP) {
        int step = (int) (input->scroll_pos / SCROLL_STEP);
        input->scroll_pos -= (float) (step) * SCROLL_STEP;
        filr_move_index(context, -step);
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

void change_file_sorting(INPUTS_ARGS) {
    result err = filr_next_sorting_ix(context);
    if (err.err)  {
        logger_setup_err(context, view, input, err);
        return;
    }
    context->visible_index = 0;
    view_center_camera(context, view);
}

void view_dotfiles(INPUTS_ARGS) {
    view->window.hide_dotfiles = !view->window.hide_dotfiles;
    result err = filr_set_hide_dotfiles(context, view->window.hide_dotfiles);
    if (err.err)  {
        logger_setup_err(context, view, input, err);
        return;
    }
    context->visible_index = 0;
    view_center_camera(context, view);
}

void input_mode_start(INPUTS_ARGS, mode_enum mode) {
    input->mode = mode;
    view_show_input(view);
    logger_mode_changed(view, input);
}

void create_file_start(INPUTS_ARGS) {
    input_mode_start(context, view, input, INPUTS_CREATE_FILE);
}

void create_directory_start(INPUTS_ARGS) {
    input_mode_start(context, view, input, INPUTS_CREATE_DIRECTORY);
}

void input_mode_cancel(INPUTS_ARGS) {
    input->mode = INPUTS_NORMAL;
    view_hide_input(view);
    cstr_init(&input->input_str, 0);
    logger_mode_changed(view, input);
}

void create_file_confirm(INPUTS_ARGS) {
    result create_err = filr_create_file(context, input->input_str);

    if (create_err.err) {
        logger_setup_err(context, view, input, create_err);
        return;
    }

    context->files_all.size = 0;
    result load_err = filr_load_directory(context);
    if (load_err.err) {
        logger_setup_err(context, view, input, load_err);
        return;
    }

    filr_move_index_filename(context, input->input_str);
    view_center_camera(context, view);

    input_mode_cancel(context, view, input);
}

void create_dir_confirm(INPUTS_ARGS) {
    result create_err = filr_create_directory(context, input->input_str);

    if (create_err.err) {
        logger_setup_err(context, view, input, create_err);
        return;
    }

    context->files_all.size = 0;
    result load_err = filr_load_directory(context);
    if (load_err.err) {
        logger_setup_err(context, view, input, load_err);
        return;
    }

    filr_move_index_filename(context, input->input_str);
    view_center_camera(context, view);

    input_mode_cancel(context, view, input);
}

void input_mode_delete_last(view_t *view, inputs_t *input) {
    cstr_pop(&input->input_str);
    view_window_set_str(&view->input, input->input_str);
}

void delete_file(INPUTS_ARGS) {
    result delete_err = filr_delete_file(context);

    if (delete_err.err) {
        logger_setup_err(context, view, input, delete_err);
        return;
    }

    context->files_all.size = 0;
    result load_err = filr_load_directory(context);
    if (load_err.err) {
        logger_setup_err(context, view, input, load_err);
    }
}


void input_mode_parse_key_queue(view_t *view, inputs_t *input) {
    int key_pressed = GetCharPressed();
    while(key_pressed) {
        if (key_pressed >= 32 && key_pressed <= 125)
            cstr_concat_single(&input->input_str, (char)key_pressed);

        key_pressed = GetCharPressed();
    }

    view_window_set_str(&view->input, input->input_str);
}

void rename_start(INPUTS_ARGS) {
    input_mode_start(context, view, input, INPUTS_RENAME);

    cstr_copy(&input->input_str, *filr_get_name_visible(context, context->visible_index));
    view_window_set_str(&view->input, input->input_str);
}

void rename_confirm(INPUTS_ARGS) {
    result rename_err = filr_rename_file(context, input->input_str);
    if (rename_err.err) {
        logger_setup_err(context, view, input, rename_err);
        return;
    }

    context->files_all.size = 0;
    result load_err = filr_load_directory(context);
    if (load_err.err) {
        logger_setup_err(context, view, input, load_err);
        return;
    }
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

            HANDLE_INPUT(file_action, context, view, input);

            HANDLE_INPUT(change_file_sorting, context, view, input);

            HANDLE_INPUT(view_dotfiles, context, view, input);

            HANDLE_INPUT(create_file_start, context, view, input);

            HANDLE_INPUT(delete_file, context, view, input);

            HANDLE_INPUT(rename_start, context, view, input);

            HANDLE_INPUT_SHIFT(create_directory_start, context, view, input);

            HANDLE_INPUT_MOUSE(mouse_left_click, context, view, input);

            HANDLE_INPUT_MOUSE_SCROLL(context, view, input);

            HANDLE_INPUT_DOWN(key_scroll_down, context, view, input);

            HANDLE_INPUT_DOWN(key_scroll_up, context, view, input);

            if (IsKeyUp(key_key_scroll_down) && IsKeyUp(key_key_scroll_up))
                input->scroll_frames_count = 0;
            break;

        case INPUTS_CREATE_FILE:
            HANDLE_INPUT(create_file_confirm, context, view, input);
            HANDLE_INPUT_CTRL(input_mode_cancel, context, view, input);
            HANDLE_INPUT(input_mode_delete_last, view, input);
            input_mode_parse_key_queue(view, input);
            break;

        case INPUTS_CREATE_DIRECTORY:
            HANDLE_INPUT(create_dir_confirm, context, view, input);
            HANDLE_INPUT_CTRL(input_mode_cancel, context, view, input);
            HANDLE_INPUT(input_mode_delete_last, view, input);
            input_mode_parse_key_queue(view, input);
            break;

        case INPUTS_RENAME:
            HANDLE_INPUT(rename_confirm, context, view, input);
            HANDLE_INPUT_CTRL(input_mode_cancel, context, view, input);
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
