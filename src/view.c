#include "../view.h"
#include "../cstr.h"

#define FONT_DIR "resources/font/firacode.ttf"
#define BG_IMG_DIR "resources/img/bg_image.png"

#define FOLDER_ICON_DIR "resources/img/icons/folder.svg"
#define FILE_ICON_DIR "resources/img/icons/file.svg"

#define PASSIVE_COLOR GetColor(0xC091ECFF)
#define HIGHLIGHT_COLOR GetColor(0xFCB3FDFF)//(0xF1D4FBFF)
#define BG_COLOR GetColor(0xB770EDFF)

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

    view.theme.folder_texture = load_svg(FOLDER_ICON_DIR, view.window.text_size);
    view.theme.file_texture = load_svg(FILE_ICON_DIR, view.window.text_size);
    

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
    return file.is_directory? theme->folder_texture : theme->file_texture;
}


cstr get_row_str(filr_file file) {
    return cstr_concat(5,
                       cstr_cap(file.name, VIEW_NAME_CAP), CSTR_SPACE,
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


        Vector2 draw_text_pos = {position.x - view.window.camera.x, position.y - view.window.camera.y};
        cstr row_str = get_row_str(context->files[ix]);

        DrawTextEx(view.theme.font,
                   row_str.str,//filr_get_name_cstr(context, ix).str,
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
