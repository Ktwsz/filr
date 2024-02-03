#ifndef VIEW_H
#define VIEW_H

#include "lib/raylib.h"
#include "filr.h"
#include "result.h"
#include "hash_map.h"


typedef struct {
    Rectangle camera;
    Vector2 text_size;
    Vector2 offset;
    bool hide_dotfiles;
    bool show;
    cstr str;
} view_window;

typedef struct {
    Color passive, highlight, bg;
    Font font;
    Texture2D bg_texture;
    hash_map file_icons;
} view_theme;

typedef struct {
    view_window window,
        header,
        input,
        logger;

    view_theme theme;
    Rectangle size;
} view_t;

typedef void(*mouse_input_callback_t)(const void *, Rectangle, int);

void view_view(filr_context *context, view_t *view, const void *inputs_ptr, mouse_input_callback_t mouse_input_callback);

void view_directory(filr_context *context, view_window *window, view_theme *theme, const void *inputs_ptr, mouse_input_callback_t mouse_input_callback);

void view_header(filr_context *context, view_window *window, view_theme *theme);

void view_logger(view_window *window, view_theme *theme);

void view_show_logger(view_t *view);

void view_hide_logger(view_t *view);

void view_show_input(view_t *view);

void view_hide_input(view_t *view);

void view_set_input_str(view_t *view, cstr str);

void view_set_logger_str(view_t *view, cstr str);

void view_input(view_window *window, view_theme *theme);

void view_center_camera(filr_context *context, view_t *view);

void view_move_camera(filr_context *context, view_t *view);

void view_handle_resize(filr_context *context, view_t *view);

void view_scroll_bar(filr_context *context, size_t ix, view_window *window, view_theme *theme);

result view_init(view_t *view, int window_width, int window_height);

void view_draw_background(view_t *view);

void view_free(view_t *view);

#endif
