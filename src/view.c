#include "../view.h"
#include "../cstr.h"

#include <math.h>

#include "theme_constants.c"

#define LOAD_SVG(a) view->theme.a##_texture = load_svg(a##_icon_dir, view->window.text_size)
#define UNLOAD(a) UnloadTexture(view->theme.a##_texture)

#define C_EXT_HASH cstr_hash("c")
#define H_EXT_HASH cstr_hash("h")
#define CPP_EXT_HASH cstr_hash("cpp") 
#define HPP_EXT_HASH cstr_hash("hpp") 
#define EXE_EXT_HASH cstr_hash("exe") 
#define HS_EXT_HASH cstr_hash("hs")
#define HTML_EXT_HASH cstr_hash("html")
#define JPG_EXT_HASH cstr_hash("jpg")
#define PNG_EXT_HASH cstr_hash("png")
#define JPEG_EXT_HASH cstr_hash("jpeg")
#define GIF_EXT_HASH cstr_hash("gif")
#define JAVA_EXT_HASH cstr_hash("java")
#define JS_EXT_HASH cstr_hash("js")
#define PDF_EXT_HASH cstr_hash("pdf")
#define PY_EXT_HASH cstr_hash("py")
#define ZIP_EXT_HASH cstr_hash("zip")


cstr CSTR_SPACE = { .str = " ", .size = 1 };

Texture load_svg(const char *src, Vector2 size) {
    Image img = LoadImageSvg(src, size.y, size.y);
    Texture t = LoadTextureFromImage(img);
    UnloadImage(img);
    return t;
}

void view_init(view_t *view, int window_width, int window_height) {
    view->size = (Rectangle) {.x = 0, .y = 0, .width = window_width, .height = window_height};
    view->show_input = false;

    view->window.offset = (Vector2) {.x = 0, .y = 40};
    view->window.camera = (Rectangle) {.x = 0, .y = 0, .width = window_width - view->window.offset.x, .height = window_height - view->window.offset.y};
    view->window.text_size = (Vector2) {.x = view->window.camera.width, .y = 30};
    view->window.hide_dotfiles = true;

    view->theme.font = LoadFontEx(FONT_DIR, 32, 0, 250);

    view->theme.passive = PASSIVE_COLOR;
    view->theme.highlight = HIGHLIGHT_COLOR;
    view->theme.bg = BG_COLOR;


    view->theme.bg_texture = LoadTexture(BG_IMG_DIR);

    LOAD_SVG(folder);
    LOAD_SVG(file);
    LOAD_SVG(c);
    LOAD_SVG(cpp);
    LOAD_SVG(exe);
    LOAD_SVG(hs);
    LOAD_SVG(html);
    LOAD_SVG(img);
    LOAD_SVG(java);
    LOAD_SVG(js);
    LOAD_SVG(pdf);
    LOAD_SVG(py);
    LOAD_SVG(zip);
}

void view_draw_background(view_t *view) {
    ClearBackground(view->theme.bg);

    int bg_pos_x = (view->size.width - view->theme.bg_texture.width) / 2;
    int bg_pos_y = (view->size.height - view->theme.bg_texture.height) / 2;

    DrawTexture(view->theme.bg_texture, bg_pos_x, bg_pos_y, RAYWHITE);
}

int max(int a, int b) {
    return (a > b)? a : b;
}

Vector2 get_position(int ix, Vector2 text_size) {
    return (Vector2){0, ix * text_size.y}; 
}

Texture get_file_icon(view_theme *theme, filr_file file) {
    if (file.is_directory) 
        return theme->folder_texture;

    cstr extension;
    cstr_strip_extension(&extension, file.name);

    int hash = cstr_hash(extension.str);
    if (C_EXT_HASH == hash)
            return theme->c_texture;
    if (H_EXT_HASH == hash)
            return theme->c_texture;
    if (CPP_EXT_HASH == hash)
            return theme->cpp_texture;
    if (HPP_EXT_HASH == hash)
            return theme->cpp_texture;
    if (EXE_EXT_HASH == hash)
            return theme->exe_texture;
    if (HS_EXT_HASH == hash)
            return theme->hs_texture;
    if (HTML_EXT_HASH == hash)
            return theme->html_texture;
    if (JPG_EXT_HASH == hash)
            return theme->img_texture;
    if (PNG_EXT_HASH == hash)
            return theme->img_texture;
    if (JPEG_EXT_HASH == hash)
            return theme->img_texture;
    if (PNG_EXT_HASH == hash)
            return theme->img_texture;
    if (JAVA_EXT_HASH == hash)
            return theme->java_texture;
    if (JS_EXT_HASH == hash)
            return theme->js_texture;
    if (PDF_EXT_HASH == hash)
            return theme->pdf_texture;
    if (PY_EXT_HASH == hash)
            return theme->py_texture;
    if (ZIP_EXT_HASH)
            return theme->zip_texture;

    return theme->file_texture;
}


void get_row_str(cstr *row, filr_file file, int name_cap) {
    cstr file_capped, file_size, date;
    cstr_cap(&file_capped, file.name, name_cap);
    cstr_parse_file_size(&file_size, file.size);
    cstr_parse_date(&date, file.last_edit_date.day,
                           file.last_edit_date.month,
                           file.last_edit_date.year,
                           file.last_edit_date.hour,
                           file.last_edit_date.minute);

    cstr_concat(row,
                5,
                file_capped, CSTR_SPACE,
                file_size, CSTR_SPACE,
                date);
}


void view_directory(filr_context *context, view_window *window, view_theme *theme, const void *inputs_ptr, mouse_input_callback_t mouse_input_callback) {
    size_t ix = ceil(window->camera.y / window->text_size.y);
    if (window->hide_dotfiles) {
        int i = 0;
        for (int ctr = 0; ctr < ix; ++ctr) {
            while (i < context->size && context->files[i].is_dotfile) i++;
        }
        ix = i;
    }

    const float padding = 40.0;
    int max_text_width = (int) ((window->camera.width - padding) / theme->font.baseSize);

    for (Vector2 position = get_position(ix, window->text_size); 
            position.y < window->camera.y + window->camera.height;
            position.y += window->text_size.y) {

        if (window->hide_dotfiles) {
            while (ix < context->size && context->files[ix].is_dotfile) ix++;
        }
        if (ix >= context->size)
            break;

        bool is_selected = context->file_index == ix;

        Rectangle row_rect = (Rectangle) {window->offset.x,
                                          window->offset.y + position.y - window->camera.y,
                                          window->text_size.x,
                                          window->text_size.y};

        if (is_selected) DrawRectangleRec(row_rect, theme->passive);

        Vector2 draw_icon_pos = {row_rect.x, row_rect.y};
        

        DrawTextureV(get_file_icon(theme, context->files[ix]),
                     draw_icon_pos,
                     RAYWHITE);


        Vector2 draw_text_pos = {row_rect.x + padding, row_rect.y};

        cstr row_str;
        get_row_str(&row_str, context->files[ix], max_text_width);

        DrawTextEx(theme->font,
                   row_str.str,
                   draw_text_pos,
                   (float)theme->font.baseSize,
                   2,
                   theme->highlight);


        mouse_input_callback(inputs_ptr, row_rect, ix);

        ix++;
    }

    view_scroll_bar(context, ix, window, theme);
}

void view_view(filr_context *context, view_t *view, const void *inputs_ptr, mouse_input_callback_t mouse_input_callback) {
    view_header(context, &view->header, &view->theme);
    view_directory(context, &view->window, &view->theme, inputs_ptr, mouse_input_callback);

    if (view->show_input)
        view_input(context, &view->input, &view->theme);
}

void view_header(filr_context *context, view_window *window, view_theme *theme) {}

void view_input(filr_context *context, view_window *window, view_theme *theme) {}

void view_center_camera(filr_context *context, view_t *view) {
    size_t ix = context->file_index;
    if (view->window.hide_dotfiles)
        ix -= filr_count_dotfiles(context, ix);

    Vector2 position = get_position(ix, view->window.text_size);

    view->window.camera.y = max(0, position.y + (view->window.text_size.y - view->window.camera.height)/2);
}

void view_move_camera(filr_context *context, view_t *view) {
    size_t ix = context->file_index;
    if (view->window.hide_dotfiles)
        ix -= filr_count_dotfiles(context, ix);

    Vector2 position = get_position(ix, view->window.text_size);

    if (position.y < view->window.camera.y)
        view->window.camera.y = position.y;

    if (position.y + view->window.text_size.y > view->window.camera.y + view->window.camera.height)
        view->window.camera.y = position.y + view->window.text_size.y - view->window.camera.height;
}

void view_handle_resize(filr_context *context, view_t *view) {
    int window_width = GetScreenWidth();
    int window_height = GetScreenHeight();

    if (window_width == view->size.width && window_height == view->size.height)
        return;

    view->size.width = window_width;
    view->size.height = window_height;

    view->window.camera.width = window_width - view->window.offset.x;
    view->window.camera.height = window_height - view->window.offset.y;
    view->window.text_size.x = view->window.camera.width;

    view_center_camera(context, view);
}

void view_scroll_bar(filr_context *context, size_t ix, view_window *window, view_theme *theme) {
    float height = ((float) ix) / (float) context->size;
    DrawRectangle(window->offset.x + window->camera.x + window->camera.width - 10, window->offset.y, 10, (int) (height * window->camera.height), theme->highlight);
}

void view_free(view_t *view) {
    UNLOAD(bg);
    UNLOAD(folder);
    UNLOAD(file);
    UNLOAD(c);
    UNLOAD(cpp);
    UNLOAD(exe);
    UNLOAD(hs);
    UNLOAD(html);
    UNLOAD(img);
    UNLOAD(java);
    UNLOAD(js);
    UNLOAD(pdf);
    UNLOAD(py);
    UNLOAD(zip);
}
