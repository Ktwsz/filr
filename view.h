#ifndef VIEW_H
#define VIEW_H

#include "lib/raylib.h"
#include "filr.h"


typedef struct {
    Rectangle camera;
    Vector2 text_size;
    Vector2 offset;
    bool hide_dotfiles;
} view_window;

typedef struct {
    Color passive, highlight, bg;
    Font font;
    Texture2D bg_texture;
    Texture2D folder_texture, file_texture, c_texture, cpp_texture, exe_texture, hs_texture, html_texture, img_texture, java_texture, js_texture, pdf_texture, py_texture, zip_texture;
} view_theme;

typedef struct {
    view_window window, header, input;
    view_theme theme;
    Rectangle size;
    bool show_input;
} view_t;

typedef void(*mouse_input_callback_t)(const void *, Rectangle, int);

void view_view(filr_context *context, view_t *view, const void *inputs_ptr, mouse_input_callback_t mouse_input_callback);

void view_directory(filr_context *context, view_window *window, view_theme *theme, const void *inputs_ptr, mouse_input_callback_t mouse_input_callback);

void view_header(filr_context *context, view_window *window, view_theme *theme);

void view_input(filr_context *context, view_window *window, view_theme *theme);

void view_center_camera(filr_context *context, view_t *view);

void view_move_camera(filr_context *context, view_t *view);

void view_handle_resize(filr_context *context, view_t *view);

void view_scroll_bar(filr_context *context, size_t ix, view_window *window, view_theme *theme);

void view_init(view_t *view, int window_width, int window_height);

void view_draw_background(view_t *view);

void view_free(view_t *view);

#endif
