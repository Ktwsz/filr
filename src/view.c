#include "../view.h"

#include <math.h>

#include "../resources/theme_blue.h"
//#include "../resources/theme_pink.h"

#define LOAD_SVG(name, file_name) t = load_svg(file_name##_icon_dir, size); \
                                  err = hash_map_insert(&theme->file_icons, #name, &t); \
                                  if (err.err) return err;

#define LOGGER_WIDTH_SMALL 160.0f
#define SCROLL_BAR_WIDTH 10.0f
#define WINDOW_PADDING_V 5.0f

cstr CSTR_SPACE = { .str = " ", .size = 1 };
cstr CSTR_FILE = { .str = "file", .size = 4 };

Texture load_svg(const char *src, Vector2 size) {
    Image img = LoadImageSvg(src, (int)size.y, (int)size.y);
    Texture t = LoadTextureFromImage(img);
    UnloadImage(img);
    return t;
}

void view_set_size_window(view_window *window, int window_width, int window_height, view_window *input) {
    window->offset = (Vector2) {.x = 0.0f, .y = 40.0f};
    window->camera.width = (float)window_width - window->offset.x;
    window->camera.height = (float)window_height - window->offset.y - 40;
    window->text_size = (Vector2) {.x = window->camera.width, .y = 30};

    if (input->show)
        window->camera.height -= input->camera.height;
}

void view_set_size_header(view_window *header, int window_width, int window_height) {
    header->offset = (Vector2) {.x = 0.0f, .y = 0.0f};
    header->camera.width = (float)window_width;
    header->camera.height = 30;
    header->text_size = (Vector2) {.x = header->camera.width, .y = header->camera.height};
}

void view_set_size_input(view_window *input, int window_width, int window_height) {
    input->offset = (Vector2) {.x = LOGGER_WIDTH_SMALL, .y = (float)window_height - 30.0f};
    input->camera.width = (float)window_width;
    input->camera.height = 30.0f;
    input->text_size = (Vector2) {.x = input->camera.width - input->offset.x, .y = input->camera.height};
}

void view_set_size_logger(view_window *logger, int window_width, int window_height) {
    logger->offset = (Vector2) {.x = 0.0f, .y = (float)window_height - 30.0f};
    logger->camera.width = LOGGER_WIDTH_SMALL;
    logger->camera.height = 30.0f;
    logger->text_size = (Vector2) {.x = logger->camera.width, .y = logger->camera.height};
}

result theme_init(view_theme *theme, Vector2 size) {
#ifdef BG_IMG_DIR
    theme->has_bg_texture = true;
    theme->bg_texture = LoadTexture(BG_IMG_DIR);
#else
    theme->has_bg_texture = false;
#endif

    theme->font = LoadFontEx(FONT_DIR, 32, 0, 250);

    theme->passive = PASSIVE_COLOR;
    theme->light= HIGHLIGHT_LIGHT_COLOR;
    theme->dark= HIGHLIGHT_DARK_COLOR;
    theme->bg = BG_COLOR;

    result err;
    err = hash_map_init(&theme->file_icons, sizeof(Texture));
    if (err.err)
        return err;

    Texture t;
    LOAD_SVG(folder, folder)
    LOAD_SVG(file, file)
    LOAD_SVG(c, c)
    LOAD_SVG(h, c)
    LOAD_SVG(cpp, cpp)
    LOAD_SVG(hpp, cpp)
    LOAD_SVG(exe, exe)
    LOAD_SVG(hs, hs)
    LOAD_SVG(html, html)
    LOAD_SVG(jpg, img)
    LOAD_SVG(png, img)
    LOAD_SVG(jpeg, img)
    LOAD_SVG(gif, img)
    LOAD_SVG(java, java)
    LOAD_SVG(js, js)
    LOAD_SVG(pdf, pdf)
    LOAD_SVG(py, py)
    LOAD_SVG(zip, zip)

    return RESULT_OK;
}

result view_init(view_t *view, int window_width, int window_height) {
    view->size = (Rectangle) {.x = 0.0f, .y = 0.0f, .width = (float)window_width, .height = (float)window_height};

    view->header = (view_window){0};
    view->header.camera = (Rectangle) {.x = 0.0f, .y = 0.0f};
    view_set_size_header(&view->header, window_width, window_height);
    view->header.show = true;
    cstr_init(&view->header.str, 0);

    view->window = (view_window){0};
    view->window.camera = (Rectangle) {.x = 0.0f, .y = 0.0f};
    view->window.hide_dotfiles = true;
    view_set_size_window(&view->window, window_width, window_height, &view->input);
    view->window.show = true;
    cstr_init(&view->window.str, 0);

    view->input = (view_window){0};
    view->input.camera = (Rectangle) {.x = 0.0f, .y = 0.0f};
    view_set_size_input(&view->input, window_width, window_height);
    view->input.show = false;
    cstr_init(&view->input.str, 0);

    view->logger = (view_window){0};
    view_set_size_logger(&view->logger, window_width, window_height);
    view->logger.show = true;
    cstr_init(&view->logger.str, 0);


    result err = theme_init(&view->theme, view->window.text_size);
    return err;

}

void view_draw_background(view_t *view) {
    ClearBackground(view->theme.bg);
    if (!view->theme.has_bg_texture)
        return;

    int bg_pos_x = ((int)view->size.width - view->theme.bg_texture.width) / 2;
    int bg_pos_y = ((int)view->size.height - view->theme.bg_texture.height) / 2;

    DrawTexture(view->theme.bg_texture, bg_pos_x, bg_pos_y, RAYWHITE);
}

int max(int a, int b) {
    return (a > b)? a : b;
}

Vector2 get_position(size_t ix, Vector2 text_size, float padding_h) {
    return (Vector2){0, (float)ix * (padding_h + text_size.y)};
}

Texture get_file_icon(view_theme *theme, filr_file file) {
    Texture *t = hash_map_get(&theme->file_icons, file.extension);
    if (t == NULL)
        t = hash_map_get(&theme->file_icons, CSTR_FILE);

    return *t;
}

void write_row_data(cstr *dst, filr_file file, int name_cap) {
    cstr file_capped, file_size, date;
    cstr_cap(&file_capped, file.name, name_cap);
    cstr_parse_file_size(&file_size, file.size);
    cstr_parse_date(&date, file.last_edit_date.day,
                    file.last_edit_date.month,
                    file.last_edit_date.year,
                    file.last_edit_date.hour,
                    file.last_edit_date.minute);

    cstr_concat(dst,
                5,
                file_capped, CSTR_SPACE,
                file_size, CSTR_SPACE,
                date);
}


float get_row_str_size(view_theme *theme, filr_file file, int name_cap) {
    cstr row;
    write_row_data(&row, file, name_cap);

    Vector2 row_size = MeasureTextEx(theme->font, row.str, (float)theme->font.baseSize, 2);
    return row_size.x;
}

int get_row_str_cap(float camera_width, view_theme *theme, filr_file file) {
    int l = -1, r = MAX_STR_LEN;

    while (r - l > 1) {
        int m = (l + r) / 2;

        float text_width = get_row_str_size(theme, file, m);
        if (text_width > camera_width) {
            r = m;
        } else {
            l = m;
        }
    }
    return l;
}

void view_show_input(view_t *view) {
    view->input.show = true;
}

void view_hide_input(view_t *view) {
    view->input.show = false;
    cstr_init(&view->input.str, 0);
}

void view_directory(filr_context *context, view_window *window, view_theme *theme, const void *inputs_ptr, mouse_input_callback_t mouse_input_callback) {
    if (!window->show)
        return;

    float ix_f = floorf(window->camera.y / (window->text_size.y + WINDOW_PADDING_V));
    int ix = (int)ix_f;

    const float padding = 40.0f;

    float icon_size = window->text_size.y;
    int row_cap = get_row_str_cap(window->text_size.x - SCROLL_BAR_WIDTH - icon_size - padding, theme, context->files_visible.files[0]);

    for (Vector2 position = get_position(ix, window->text_size, WINDOW_PADDING_V);
            position.y < window->camera.y + window->camera.height && ix < context->files_visible.size;
            position.y += window->text_size.y + WINDOW_PADDING_V, ++ix) {


        bool is_selected = context->visible_index == ix;

        Rectangle row_rect = { .x = window->offset.x,
                               .y = window->offset.y + position.y - window->camera.y,
                               .width = window->text_size.x - SCROLL_BAR_WIDTH,
                               .height = window->text_size.y };

        if (is_selected) DrawRectangleRec(row_rect, theme->passive);

        Vector2 draw_icon_pos = {row_rect.x, row_rect.y};
        

        DrawTextureV(get_file_icon(theme, context->files_visible.files[ix]),
                     draw_icon_pos,
                     RAYWHITE);


        Vector2 draw_text_pos = {row_rect.x + padding, row_rect.y};

        cstr row_str;
        write_row_data(&row_str, context->files_visible.files[ix], row_cap);

        Color color = (context->files_visible.files[ix].is_directory)? theme->dark: theme->light;
        DrawTextEx(theme->font,
                   row_str.str,
                   draw_text_pos,
                   (float)theme->font.baseSize,
                   2,
                   color);


        mouse_input_callback(inputs_ptr, row_rect, ix);
    }

    view_scroll_bar(context, ix, window, theme);
}

void view_view(filr_context *context, view_t *view, const void *inputs_ptr, mouse_input_callback_t mouse_input_callback) {
    view_header(context, &view->header, &view->theme);
    view_directory(context, &view->window, &view->theme, inputs_ptr, mouse_input_callback);

    view_input(&view->input, &view->theme);
    view_logger(&view->logger, &view->theme);
}

void view_header(filr_context *context, view_window *header, view_theme *theme) {
    if (!header->show)
        return;

    Rectangle header_rect = {.x = header->offset.x + header->camera.x,
                             .y = header->offset.y + header->camera.y,
                             .width = header->camera.width,
                             .height = header->camera.height};

    DrawRectangleRec(header_rect, theme->passive);

    DrawTextEx(theme->font,
               context->directory.str,
               (Vector2){header_rect.x, header_rect.y},
               (float)theme->font.baseSize,
               2,
               theme->dark);
}

void view_input(view_window *window, view_theme *theme) {
    if (!window->show)
        return;

    Rectangle input_rect = {.x = window->offset.x,
                            .y = window->offset.y,
                            .width = window->camera.width,
                            .height = window->camera.height};

    DrawRectangleRec(input_rect, theme->passive);
    DrawTextEx(theme->font,
               window->str.str,
               (Vector2){input_rect.x, input_rect.y},
               (float)theme->font.baseSize,
               2,
               theme->light);
}

void view_logger(view_window *window, view_theme *theme) {
    if (!window->show)
        return;

    Rectangle logger_rect = {.x = window->offset.x,//TODO: move to separate function
                            .y = window->offset.y,
                            .width = window->camera.width,
                            .height = window->camera.height};

    DrawRectangleRec(logger_rect, theme->passive);
    DrawTextEx(theme->font,//TODO: this also perhaps
               window->str.str,
               (Vector2){logger_rect.x, logger_rect.y},
               (float)theme->font.baseSize,
               2,
               theme->light);
}

void view_center_camera(filr_context *context, view_t *view) {
    size_t ix = context->visible_index;

    Vector2 position = get_position(ix, view->window.text_size, WINDOW_PADDING_V);

    view->window.camera.y = max(0, position.y + (view->window.text_size.y - view->window.camera.height)/2);
}

void view_move_camera(filr_context *context, view_t *view) {
    size_t ix = context->visible_index;

    Vector2 position = get_position(ix, view->window.text_size, WINDOW_PADDING_V);

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

    view_set_size_header(&view->header, window_width, window_height);

    view_set_size_window(&view->window, window_width, window_height, &view->input);

    view_set_size_input(&view->input, window_width, window_height);

    view_set_size_logger(&view->logger, window_width, window_height);

    view_center_camera(context, view);
}

void view_window_set_str(view_window *window, cstr str) {
    cstr_copy(&window->str, str);
}


void view_logger_set_err(view_t *view, result err) {
    view->logger.camera.width = view->size.width;

    cstr_init_name(&view->logger.str, err.message);
}

void view_logger_clear_err(view_t *view) {
    view->logger.camera.width = LOGGER_WIDTH_SMALL;
}

void view_scroll_bar(filr_context *context, size_t ix, view_window *window, view_theme *theme) {
    float height = ((float) ix) / (float) context->files_visible.size;
    DrawRectangle(window->offset.x + window->camera.x + window->camera.width - SCROLL_BAR_WIDTH, window->offset.y, SCROLL_BAR_WIDTH, (int) (height * window->camera.height), theme->dark);
}

void view_theme_free(view_theme *theme) {
    hash_map_free(&theme->file_icons);
}

void view_free(view_t *view) {
    view_theme_free(&view->theme);
}
