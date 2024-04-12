#include "inputs.h"

#include <math.h>
#include "../config/inputs.h"


#define ARGS filr_context *context, view_t *view, inputs_t *input
#define PASS_ARGS context, view, input

#define CONTEXT_FOCUS context[input->window_focus]


#define HANDLE_INPUT(input, ...) if (!IsKeyDown(KEY_LEFT_SHIFT) && !IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(key_##input)) input(__VA_ARGS__)
#define HANDLE_INPUT_SHIFT(input, ...) if (IsKeyDown(KEY_LEFT_SHIFT) && !IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(key_##input)) input(__VA_ARGS__)
#define HANDLE_INPUT_CTRL(input, ...) if (!IsKeyDown(KEY_LEFT_SHIFT) && IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(key_##input)) input(__VA_ARGS__)
#define HANDLE_INPUT_MOUSE(input, ...) if (IsMouseButtonPressed(key_##input)) input(__VA_ARGS__)
#define HANDLE_INPUT_MOUSE_SCROLL(...) mouse_scroll(__VA_ARGS__)
#define HANDLE_INPUT_DOWN(input, ...) if (IsKeyDown(key_##input)) input(__VA_ARGS__)

cstr mode_bar[] = {
    { .str = "NORMAL |", .size = 8 },
    { .str = "FILE   |", .size = 8 },
    { .str = "DIR    |", .size = 8 },
    { .str = "RENAME |", .size = 8 },
    { .str = "MOVE   |", .size = 8 },
    { .str = "COPY   |", .size = 8 },
};


void logger_mode_changed(view_t *view, inputs_t *input);

void inputs_init(inputs_t *input, view_t *view) {
    input->mouse_ix = -1;
    input->mouse_focus = 0;
    input->mouse_ix_rect = (Rectangle){0};
    input->scroll_pos = 0;
    input->mode = INPUTS_NORMAL;
    input->scroll_frames_count = 0;
    input->window_focus = 0;
    cstr_init(&input->input_str, 0);

    logger_mode_changed(view, input);
}


void mode_cancel(ARGS);

void logger_setup_err(ARGS, result err) {
    view_logger_set_err(view, err);
    if (input->mode == INPUTS_RENAME || input->mode == INPUTS_CREATE_FILE || input->mode == INPUTS_CREATE_DIRECTORY)
        mode_cancel(&CONTEXT_FOCUS, view, input);
}

void logger_mode_changed(view_t *view, inputs_t *input) {
    view_logger_clear_err(view);
    
    view_window_set_str(&view->logger, mode_bar[input->mode]);
}

void center_camera(ARGS) {
    view_center_camera(&CONTEXT_FOCUS, view, input->window_focus);
}

void move_one_down(ARGS) {
    filr_move_index(&CONTEXT_FOCUS, 1);
    view_move_camera(&CONTEXT_FOCUS, view, input->window_focus);
}

void move_one_up(ARGS) {
    filr_move_index(&CONTEXT_FOCUS, -1);
    view_move_camera(&CONTEXT_FOCUS, view, input->window_focus);
}

void file_action(ARGS) {
    bool dir_change = CONTEXT_FOCUS.files_visible.files[CONTEXT_FOCUS.visible_index].is_directory;
    result err = filr_action(&CONTEXT_FOCUS);
    if (err.err) {
        logger_setup_err(context, view, input, err);
        return;
    }

    if (dir_change) {
        view_center_camera(&CONTEXT_FOCUS, view, input->window_focus);
    }
}

void mouse_left_click(ARGS) {
    if (CONTEXT_FOCUS.visible_index == input->mouse_ix) {
        Vector2 mouse_position = GetMousePosition();
        if (CheckCollisionPointRec(mouse_position, input->mouse_ix_rect))
            file_action(context, view, input);
    } else if (input->mouse_ix != -1) {
        input->window_focus = input->mouse_focus;
        CONTEXT_FOCUS.visible_index = input->mouse_ix;
    }
}


void scroll(ARGS, float scroll_move) {
    input->scroll_pos += (scroll_move > SCROLL_SPEED_CAP)? SCROLL_SPEED_CAP : scroll_move;
    if (fabs(input->scroll_pos) > SCROLL_STEP) {
        int step = (int) (input->scroll_pos / SCROLL_STEP);
        input->scroll_pos -= (float) (step) * SCROLL_STEP;
        filr_move_index(&CONTEXT_FOCUS, -step);
        view_center_camera(&CONTEXT_FOCUS, view, input->window_focus);
    }
}


void mouse_scroll(ARGS) {
    float mouse_wheel_move = 0;

    if ((mouse_wheel_move = GetMouseWheelMove())) {
        scroll(context, view, input, mouse_wheel_move);
    }
}

void key_scroll_down(ARGS) {
    if (input->scroll_frames_count < SCROLL_FRAME_THRESHOLD)
        input->scroll_frames_count++;
    else
        scroll(context, view, input, -SCROLL_KEY_SPEED);
}

void key_scroll_up(ARGS) {
    if (input->scroll_frames_count < SCROLL_FRAME_THRESHOLD)
        input->scroll_frames_count++;
    else
        scroll(context, view, input, SCROLL_KEY_SPEED);
}

void change_file_sorting(ARGS) {
    result err = filr_next_sorting_ix(&CONTEXT_FOCUS);
    if (err.err)  {
        logger_setup_err(context, view, input, err);
        return;
    }
    CONTEXT_FOCUS.visible_index = 0;
    view_center_camera(&CONTEXT_FOCUS, view, input->window_focus);
}

void view_dotfiles(ARGS) {
    result err;
    if (input->window_focus == 0) {
        view->window.hide_dotfiles = !view->window.hide_dotfiles;
        err = filr_set_hide_dotfiles(&CONTEXT_FOCUS, view->window.hide_dotfiles);
    } else {
        view->second_window.hide_dotfiles = !view->second_window.hide_dotfiles;
        err = filr_set_hide_dotfiles(&CONTEXT_FOCUS, view->second_window.hide_dotfiles);
    }

    if (err.err)  {
        logger_setup_err(context, view, input, err);
        return;
    }
    CONTEXT_FOCUS.visible_index = 0;
    view_center_camera(&CONTEXT_FOCUS, view, input->window_focus);
}

void input_mode_start(ARGS, mode_enum mode) {
    input->mode = mode;
    view_show_input(view);
    logger_mode_changed(view, input);
}

void create_file_start(ARGS) {
    input_mode_start(context, view, input, INPUTS_CREATE_FILE);
}

void create_directory_start(ARGS) {
    input_mode_start(context, view, input, INPUTS_CREATE_DIRECTORY);
}

void mode_cancel(ARGS) {
    input->mode = INPUTS_NORMAL;
    view_hide_input(view);
    cstr_init(&input->input_str, 0);
    logger_mode_changed(view, input);
}

void create_file_confirm(ARGS) {
    result create_err = filr_create_file(&CONTEXT_FOCUS, input->input_str);

    if (create_err.err) {
        logger_setup_err(context, view, input, create_err);
        return;
    }

    CONTEXT_FOCUS.files_all.size = 0;
    result load_err = filr_load_directory(&CONTEXT_FOCUS);
    if (load_err.err) {
        logger_setup_err(context, view, input, load_err);
        return;
    }

    filr_move_index_filename(&CONTEXT_FOCUS, input->input_str);
    view_center_camera(&CONTEXT_FOCUS, view, input->window_focus);

    mode_cancel(context, view, input);
}

void create_directory_confirm(ARGS) {
    result create_err = filr_create_directory(&CONTEXT_FOCUS, input->input_str);

    if (create_err.err) {
        logger_setup_err(context, view, input, create_err);
        return;
    }

    CONTEXT_FOCUS.files_all.size = 0;
    result load_err = filr_load_directory(&CONTEXT_FOCUS);
    if (load_err.err) {
        logger_setup_err(context, view, input, load_err);
        return;
    }

    filr_move_index_filename(&CONTEXT_FOCUS, input->input_str);
    view_center_camera(&CONTEXT_FOCUS, view, input->window_focus);

    mode_cancel(context, view, input);
}

void input_mode_delete_last(view_t *view, inputs_t *input) {
    cstr_pop(&input->input_str);
    view_window_set_str(&view->input, input->input_str);
}

void delete_file(ARGS) {
    result delete_err = filr_delete_file(&CONTEXT_FOCUS);

    if (delete_err.err) {
        logger_setup_err(context, view, input, delete_err);
        return;
    }

    CONTEXT_FOCUS.files_all.size = 0;
    result load_err = filr_load_directory(&CONTEXT_FOCUS);
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

void rename_start(ARGS) {
    input_mode_start(context, view, input, INPUTS_RENAME);

    cstr_copy(&input->input_str, *filr_get_name_visible(&CONTEXT_FOCUS, CONTEXT_FOCUS.visible_index));
    view_window_set_str(&view->input, input->input_str);
}

void rename_confirm(ARGS) {
    result rename_err = filr_rename_file(&CONTEXT_FOCUS, input->input_str);
    if (rename_err.err) {
        logger_setup_err(context, view, input, rename_err);
        return;
    }

    CONTEXT_FOCUS.files_all.size = 0;
    result load_err = filr_load_directory(&CONTEXT_FOCUS);
    if (load_err.err) {
        logger_setup_err(context, view, input, load_err);
        return;
    }
    filr_move_index_filename(&CONTEXT_FOCUS, input->input_str);
    view_center_camera(&CONTEXT_FOCUS, view, input->window_focus);

    mode_cancel(context, view, input);
}

void open_windows_explorer(ARGS) {
    result err = filr_open_windows_explorer(&CONTEXT_FOCUS);

    if (err.err)
        logger_setup_err(context, view, input, err);
}

void open_nvim(ARGS) {
    result err = filr_open_nvim(&CONTEXT_FOCUS);

    if (err.err)
        logger_setup_err(context, view, input, err);
}

void toggle_second_window(ARGS) {
    view_toggle_second_window(view);
    if (!view->second_window.show)
        input->window_focus = 0;
}

void change_window_focus_left(ARGS) {
    if (!view->second_window.show)
        return;
    input->window_focus = 0;
}

void change_window_focus_right(ARGS) {
    if (!view->second_window.show)
        return;
    input->window_focus = 1;
}

void toggle_file_data(ARGS) {
    view_toggle_hide_file_data(view, input->window_focus);
}

void select_file(ARGS) {
    filr_select_clear(&context[1 - input->window_focus]);

    result err = filr_select_toggle_file(&CONTEXT_FOCUS);

    if (err.err)
        logger_setup_err(context, view, input, err);
}

void select_all(ARGS) {
    filr_select_toggle_all(&CONTEXT_FOCUS);
}

void select_clear(ARGS) {
    filr_select_clear(&context[0]);
    filr_select_clear(&context[1]);
}

void move_start(ARGS) {
    input_mode_start(context, view, input, INPUTS_MOVE);
    filr_select_buffer_save(&CONTEXT_FOCUS);

    input->file_buffer = input->window_focus;
}

void copy_start(ARGS) {
    input_mode_start(context, view, input, INPUTS_COPY);
    filr_select_buffer_save(&CONTEXT_FOCUS);

    input->file_buffer = input->window_focus;
}

void copy_confirm(ARGS) {
    result err = filr_select_buffer_copy(&context[input->file_buffer], CONTEXT_FOCUS.directory);
    if (err.err)
        logger_setup_err(context, view, input, err);

    filr_select_clear(&context[input->file_buffer]);
    filr_select_buffer_clear(&context[input->file_buffer]);
}

void move_confirm(ARGS) {
    result err = filr_select_buffer_move(&context[input->file_buffer], CONTEXT_FOCUS.directory);
    if (err.err)
        logger_setup_err(context, view, input, err);

    filr_select_clear(&context[input->file_buffer]);
    filr_select_buffer_clear(&context[input->file_buffer]);
}

void handle_key_presses(ARGS) {
    switch (input->mode) {
        case INPUTS_NORMAL:
            HANDLE_INPUT(create_file_start, PASS_ARGS);

            HANDLE_INPUT(delete_file, PASS_ARGS);

            HANDLE_INPUT(rename_start, PASS_ARGS);

            HANDLE_INPUT(open_windows_explorer, PASS_ARGS);

            HANDLE_INPUT(open_nvim, PASS_ARGS);

            HANDLE_INPUT_SHIFT(create_directory_start, PASS_ARGS);

            HANDLE_INPUT(select_file, PASS_ARGS);

            HANDLE_INPUT_SHIFT(select_all, PASS_ARGS);

            HANDLE_INPUT_CTRL(select_clear, PASS_ARGS);

            HANDLE_INPUT(move_start, PASS_ARGS);

            HANDLE_INPUT(copy_start, PASS_ARGS);

movement_binds:
            HANDLE_INPUT(center_camera, PASS_ARGS);

            HANDLE_INPUT(move_one_down, PASS_ARGS);

            HANDLE_INPUT(move_one_up, PASS_ARGS);

            HANDLE_INPUT(file_action, PASS_ARGS);

            HANDLE_INPUT(change_file_sorting, PASS_ARGS);

            HANDLE_INPUT(view_dotfiles, PASS_ARGS);

            HANDLE_INPUT(toggle_second_window, PASS_ARGS);

            HANDLE_INPUT(change_window_focus_left, PASS_ARGS);

            HANDLE_INPUT(change_window_focus_right, PASS_ARGS);

            HANDLE_INPUT(toggle_file_data, PASS_ARGS);


            HANDLE_INPUT_MOUSE(mouse_left_click, PASS_ARGS);

            HANDLE_INPUT_MOUSE_SCROLL(PASS_ARGS);

            HANDLE_INPUT_DOWN(key_scroll_down, PASS_ARGS);

            HANDLE_INPUT_DOWN(key_scroll_up, PASS_ARGS);

            if (IsKeyUp(key_key_scroll_down) && IsKeyUp(key_key_scroll_up))
                input->scroll_frames_count = 0;
            break;

        case INPUTS_CREATE_FILE:
            HANDLE_INPUT(create_file_confirm, PASS_ARGS);
            HANDLE_INPUT_CTRL(mode_cancel, PASS_ARGS);
            HANDLE_INPUT(input_mode_delete_last, view, input);
            input_mode_parse_key_queue(view, input);
            break;

        case INPUTS_CREATE_DIRECTORY:
            HANDLE_INPUT(create_directory_confirm, PASS_ARGS);
            HANDLE_INPUT_CTRL(mode_cancel, PASS_ARGS);
            HANDLE_INPUT(input_mode_delete_last, view, input);
            input_mode_parse_key_queue(view, input);
            break;

        case INPUTS_RENAME:
            HANDLE_INPUT(rename_confirm, PASS_ARGS);
            HANDLE_INPUT_CTRL(mode_cancel, PASS_ARGS);
            HANDLE_INPUT(input_mode_delete_last, view, input);
            input_mode_parse_key_queue(view, input);
            break;

        case INPUTS_COPY: 
            HANDLE_INPUT(copy_confirm, PASS_ARGS);
            HANDLE_INPUT_CTRL(mode_cancel, PASS_ARGS);

            goto movement_binds;

            break;

        case INPUTS_MOVE: 
            HANDLE_INPUT(move_confirm, PASS_ARGS);
            HANDLE_INPUT_CTRL(mode_cancel, PASS_ARGS);

            goto movement_binds;

            break;

    }
}

void mouse_input_callback(const void *inputs_ptr, Rectangle rect, int ix, int focus) {
    inputs_t *input = (inputs_t*)inputs_ptr;

    Vector2 mouse_position = GetMousePosition();

    if (CheckCollisionPointRec(mouse_position, rect)) {
        input->mouse_ix = ix;
        input->mouse_focus = focus;
        input->mouse_ix_rect = rect;
    }

}

//TODO: add fuzzy finder
//TODO: add cd command
