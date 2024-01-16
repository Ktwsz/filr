#include "../view.h"
#include "../cstr.h"

#include "theme_constants.c"

#define LOAD_SVG(a) view.theme.a##_texture = load_svg(a##_icon_dir, view.window.text_size)

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

#define VIEW_NAME_CAP 40

cstr CSTR_SPACE = { .str = " ", .size = 1 };

Texture load_svg(const char *src, Vector2 size) {
    Image img = LoadImageSvg(src, size.y, size.y);
    Texture t = LoadTextureFromImage(img);
    UnloadImage(img);
    return t;
}

view_t view_init(int window_width, int window_height) {
    view_t view = {0};


    view.window.camera = (Rectangle) {.x = 0, .y = 0, .width = window_width, .height = window_height};
    view.window.offset = (Vector2) {.x = 30, .y = 10};
    view.window.text_size = (Vector2) {.x = window_width-30, .y = 30};

    view.theme.font = LoadFontEx(FONT_DIR, 32, 0, 250);

    view.theme.passive = PASSIVE_COLOR;
    view.theme.highlight = HIGHLIGHT_COLOR;
    view.theme.bg = BG_COLOR;


    view.theme.bg_texture = LoadTexture(BG_IMG_DIR);

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
    
    return view;
}

void view_draw_background(view_t view) {
    ClearBackground(view.theme.bg);

    int bg_pos_x = (view.window.camera.width - view.theme.bg_texture.width) / 2;
    int bg_pos_y = (view.window.camera.height - view.theme.bg_texture.height) / 2;

    DrawTexture(view.theme.bg_texture, bg_pos_x, bg_pos_y, RAYWHITE);
}

int max(int a, int b) {
    return (a > b)? a : b;
}

Vector2 get_position(int ix, Vector2 text_size, Vector2 offset) {
    return (Vector2) {offset.x, (float) (offset.y + text_size.y * ix)};
}

Texture get_file_icon(view_theme *theme, filr_file file) {
    if (file.is_directory) 
        return theme->folder_texture;

    cstr extension = cstr_strip_extension(file.name);
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


cstr get_row_str(filr_file file, int name_cap) {
    return cstr_concat(5,
                       cstr_cap(file.name, name_cap), CSTR_SPACE,
                       cstr_parse_file_size(file.size), CSTR_SPACE,
                       cstr_parse_date(file.last_edit_date.day,
                                       file.last_edit_date.month,
                                       file.last_edit_date.year,
                                       file.last_edit_date.hour,
                                       file.last_edit_date.minute));
}


int view_directory_contents(filr_context *context, view_t view) {
    size_t ix = (view.window.camera.y - view.window.offset.y) / view.window.text_size.y;
    int mouse_ix = -1;
    Vector2 mouse_position = GetMousePosition();

    const int padding = 5;
    int window_width = view.window.text_size.x;
    int max_text_width = (window_width - 22) / view.theme.font.baseSize;
    if (max_text_width > VIEW_NAME_CAP) max_text_width = VIEW_NAME_CAP;

    for (; ix < context->size; ++ix) {
        Vector2 position = get_position(ix, view.window.text_size, view.window.offset);
        if (position.y >= view.window.camera.y + view.window.camera.height) break;

        bool is_selected = context->file_index == ix;

        Rectangle row_rect = (Rectangle) {0,
                                          position.y - view.window.camera.y,
                                          view.window.text_size.x,
                                          view.window.text_size.y};

        if (is_selected) DrawRectangleRec(row_rect, view.theme.passive);

        Vector2 draw_icon_pos = {0, position.y - view.window.camera.y};

        DrawTextureV(get_file_icon(&view.theme, context->files[ix]),
                     draw_icon_pos,
                     RAYWHITE);


        Vector2 draw_text_pos = {position.x - view.window.camera.x + padding, position.y - view.window.camera.y};
        cstr row_str = get_row_str(context->files[ix], max_text_width);

        DrawTextEx(view.theme.font,
                   row_str.str,
                   draw_text_pos,
                   (float)view.theme.font.baseSize,
                   2,
                   view.theme.highlight);


        
        if (CheckCollisionPointRec(mouse_position, row_rect))
            mouse_ix = ix;
    }

    view_scroll_bar(context, ix, view);

    for (Vector2 position = get_position(ix, view.window.text_size, view.window.offset); 
            position.y < view.window.camera.y + view.window.camera.height; position.y += view.window.text_size.y) {
        DrawTextEx(view.theme.font,
                   "~",
                   position,
                   (float)view.theme.font.baseSize,
                   2,
                   view.theme.highlight);
    }

    return mouse_ix;
}

void view_center_camera(filr_context *context, view_t *view) {
    size_t ix = context->file_index;
    Vector2 position = get_position(ix, view->window.text_size, view->window.offset);

    view->window.camera.y = max(0, position.y + (view->window.text_size.y - view->window.camera.height)/2);
}

void view_move_camera(filr_context *context, view_t *view) {
    size_t ix = context->file_index;
    Vector2 position = get_position(ix, view->window.text_size, view->window.offset);

    if (position.y < view->window.camera.y)
        view->window.camera.y = position.y;

    if (position.y + view->window.text_size.y > view->window.camera.y + view->window.camera.height)
        view->window.camera.y = position.y + view->window.text_size.y - view->window.camera.height;
}

void view_resize(view_t *view, int window_width, int window_height) {
    view->window.camera.width = window_width;
    view->window.camera.height = window_height;

    view->window.text_size.x = window_width - view->window.offset.x;
}

void view_scroll_bar(filr_context *context, size_t ix, view_t view) {
    float height = ((float) ix) / (float) context->size;
    DrawRectangle(view.window.camera.x + view.window.camera.width - 10, 0, 10, (int) (height * view.window.camera.height), view.theme.highlight);
}

void view_free(view_t view) {
    UnloadTexture(view.theme.bg_texture);
    UnloadTexture(view.theme.folder_texture);
}
