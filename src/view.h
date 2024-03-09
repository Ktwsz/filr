#ifndef VIEW_H
#define VIEW_H

#include "raylib.h"
#include "filr.h"
#include "result.h"
#include "hash_map.h"


typedef struct {
    Rectangle camera;
    Vector2 text_size;
    Vector2 offset;
    bool hide_dotfiles;
    bool hide_file_data;
    bool show;
    cstr str;
    int file_display_row_cap;
} view_window;

typedef struct {
    Color passive, light, bg, dark;
    Font font;
    Texture2D bg_texture;
    hash_map file_icons;
    bool has_bg_texture;
} view_theme;

typedef struct {
    view_window window,
        second_window,
        header,
        second_header,
        input,
        logger;
    view_theme theme;
    Rectangle size;
} view_t;

typedef void(*mouse_input_callback_t)(const void *, Rectangle, int, int);

void view_view(filr_context *context, view_t *view, const void *inputs_ptr, mouse_input_callback_t mouse_input_callback, int window_focus);

void view_directory(filr_context *context, view_window *window, view_theme *theme, const void *inputs_ptr, mouse_input_callback_t mouse_input_callback, int window_id, bool is_focused);

void view_header(filr_context *context, view_window *window, view_theme *theme);

void view_logger(view_window *window, view_theme *theme);

void view_toggle_second_window(view_t *window);

void view_toggle_hide_file_data(view_t *view, int window_id);

void view_show_input(view_t *view);

void view_hide_input(view_t *view);

void view_window_set_str(view_window *window, cstr str);

void view_logger_set_err(view_t *view, result err);

void view_logger_clear_err(view_t *view);

void view_input(view_window *window, view_theme *theme);

void view_center_camera(filr_context *context, view_t *view, int window_focus);

void view_move_camera(filr_context *context, view_t *view, int window_focus);

void view_handle_resize(filr_context *context, view_t *view, int window_focus);

void view_scroll_bar(filr_context *context, size_t ix, view_window *window, view_theme *theme);

result view_init(view_t *view, float window_width, float window_height);

void view_draw_background(view_t *view);

void view_free(view_t *view);

#endif
