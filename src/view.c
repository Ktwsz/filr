#include "../view.h"

#include <math.h>

#include "../resources/theme_blue.h"
//#include "../resources/theme_pink.h"

#define LOAD_SVG(name, file_name) t = load_svg(file_name##_icon_dir, size); \
                                  err = hash_map_insert(&theme->file_icons, #name, &t); \
                                  if (err.err) return err;

#define LOGGER_WIDTH 600.0f
#define LOGGER_HEIGHT 600.0f


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
    window->camera.height = (float)window_height - window->offset.y;
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
    input->offset = (Vector2) {.x = 0.0f, .y = (float)window_height - 30.0f};
    input->camera.width = (float)window_width;
    input->camera.height = 30.0f;
    input->text_size = (Vector2) {.x = input->camera.width, .y = input->camera.height};
}

void view_set_size_logger(view_window *logger, int window_width, int window_height) {
    float pos_x = ((float)window_width - LOGGER_WIDTH) / 2;
    float pos_y = ((float)window_height - LOGGER_HEIGHT) / 2;

    logger->offset = (Vector2) {.x = pos_x, .y = pos_y};
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
    view->logger.camera = (Rectangle) {.x = 0.0f, .y = 0.0f, .width = LOGGER_WIDTH, .height = LOGGER_HEIGHT};
    view->logger.text_size = (Vector2) {.x = LOGGER_WIDTH, .y = LOGGER_HEIGHT};
    view_set_size_logger(&view->logger, window_width, window_height);
    view->logger.show = false;
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

Vector2 get_position(size_t ix, Vector2 text_size) {
    return (Vector2){0, (float)ix * text_size.y};
}

Texture get_file_icon(view_theme *theme, filr_file file) {
    Texture *t = hash_map_get(&theme->file_icons, file.extension);
    if (t == NULL)
        t = hash_map_get(&theme->file_icons, CSTR_FILE);

    return *t;
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

void view_show_input(view_t *view) {
    view->input.show = true;
    view->window.camera.height -= view->input.camera.height;
}

void view_hide_input(view_t *view) {
    view->input.show = false;
    cstr_init(&view->input.str, 0);
    view->window.camera.height += view->input.camera.height;
}

void view_show_logger(view_t *view) {
    view->logger.show = true;
}

void view_hide_logger(view_t *view) {
    view->logger.show = false;
    cstr_init(&view->logger.str, 0);
}


void view_directory(filr_context *context, view_window *window, view_theme *theme, const void *inputs_ptr, mouse_input_callback_t mouse_input_callback) {
    if (!window->show)
        return;

    float ix_f = ceilf(window->camera.y / window->text_size.y);
    int draw_ix = (int)ix_f;
    int file_ix = 0;

    if (window->hide_dotfiles) {
        for (int ctr = 0; ctr < draw_ix; ++ctr, ++file_ix) {
            while (file_ix + 1 < context->size && context->files[file_ix + 1].is_dotfile) file_ix++;
        }
    }

    const float padding = 40.0;
    int max_text_width = (int) ((window->camera.width - padding) / theme->font.baseSize);

    for (Vector2 position = get_position(draw_ix, window->text_size); 
            position.y < window->camera.y + window->camera.height;
            position.y += window->text_size.y) {

        if (window->hide_dotfiles) {
            while (file_ix < context->size && context->files[file_ix].is_dotfile) file_ix++;
        }
        if (file_ix >= context->size)
            break;

        bool is_selected = context->file_index == file_ix;

        Rectangle row_rect = { .x = window->offset.x,
                               .y = window->offset.y + position.y - window->camera.y,
                               .width = window->text_size.x,
                               .height = window->text_size.y };

        if (is_selected) DrawRectangleRec(row_rect, theme->passive);

        Vector2 draw_icon_pos = {row_rect.x, row_rect.y};
        

        DrawTextureV(get_file_icon(theme, context->files[file_ix]),
                     draw_icon_pos,
                     RAYWHITE);


        Vector2 draw_text_pos = {row_rect.x + padding, row_rect.y};

        cstr row_str;
        get_row_str(&row_str, context->files[file_ix], max_text_width);

        Color color = (context->files[file_ix].is_directory)? theme->dark: theme->light;
        DrawTextEx(theme->font,
                   row_str.str,
                   draw_text_pos,
                   (float)theme->font.baseSize,
                   2,
                   color);


        mouse_input_callback(inputs_ptr, row_rect, file_ix);

        file_ix++;
    }

    view_scroll_bar(context, file_ix, window, theme);
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
               (Vector2){window->offset.x, window->offset.y + window->camera.height / 2},
               (float)theme->font.baseSize / 1.5f,
               2,
               theme->light);
}

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

    view_set_size_header(&view->header, window_width, window_height);

    view_set_size_window(&view->window, window_width, window_height, &view->input);

    view_set_size_input(&view->input, window_width, window_height);

    view_set_size_logger(&view->logger, window_width, window_height);

    view_center_camera(context, view);
}

void view_set_input_str(view_t *view, cstr str) {
    cstr_copy(&view->input.str, str);
}

void view_set_logger_str(view_t *view, cstr str) {
    cstr_copy(&view->logger.str, str);
}

void view_scroll_bar(filr_context *context, size_t ix, view_window *window, view_theme *theme) {
    float height = ((float) ix) / (float) context->size;
    DrawRectangle(window->offset.x + window->camera.x + window->camera.width - 10, window->offset.y, 10, (int) (height * window->camera.height), theme->dark);
}

void view_theme_free(view_theme *theme) {
    hash_map_free(&theme->file_icons);
}

void view_free(view_t *view) {
    view_theme_free(&view->theme);
}
