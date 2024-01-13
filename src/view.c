#include "../view.h"

int max(int a, int b) {
    return (a > b)? a : b;
}

Vector2 get_position(int ix, Vector2 text_rect, Vector2 offset) {
    return (Vector2) {offset.x, (float) (offset.y + text_rect.y * ix)};
}


int view_directory_contents(filr_context *context, Rectangle view_camera, Vector2 text_rect, Vector2 offset, Font *font) {
    size_t ix = (view_camera.y - offset.y) / text_rect.y;
    int mouse_ix = -1;
    Vector2 mouse_position = GetMousePosition();

    for (; ix < context->size; ++ix) {
        Vector2 position = get_position(ix, text_rect, offset);
        if (position.y >= view_camera.y + view_camera.height) break;

        Color highlight_color = RAYWHITE;
        if (context->file_index == ix) highlight_color = GREEN;

        Vector2 draw_rect = {position.x - view_camera.x, position.y - view_camera.y};

        DrawTextEx(*font,
                   filr_get_file_name(context, ix),
                   draw_rect,
                   (float)font->baseSize,
                   2,
                   highlight_color);

        if (CheckCollisionPointRec(mouse_position, (Rectangle) {draw_rect.x, draw_rect.y, draw_rect.x + text_rect.x, draw_rect.y + text_rect.y}))
            mouse_ix = ix;
    }

    view_scroll_bar(context, ix, view_camera);

    for (Vector2 position = get_position(ix, text_rect, offset); position.y < view_camera.y + view_camera.height; position.y += text_rect.y) {
        DrawTextEx(*font,
                   "~",
                   position,
                   (float)font->baseSize,
                   2,
                   RAYWHITE);
    }

    return mouse_ix;
}

void view_center_camera(filr_context *context, Rectangle *view_camera, Vector2 text_rect, Vector2 offset) {
    size_t ix = context->file_index;
    Vector2 position = get_position(ix, text_rect, offset);

    view_camera->y = max(0, position.y + (text_rect.y - view_camera->height)/2);
}

void view_move_camera(filr_context *context, Rectangle *view_camera, Vector2 text_rect, Vector2 offset) {
    size_t ix = context->file_index;
    Vector2 position = get_position(ix, text_rect, offset);

    if (position.y < view_camera->y)
        view_camera->y = position.y;

    if (position.y + text_rect.y > view_camera->y + view_camera->height)
        view_camera->y = position.y + text_rect.y - view_camera->height;
}

void view_resize_rects(Rectangle *view_camera, Vector2 *text_rect, Vector2 offset, int window_width, int window_height) {
    view_camera->width = window_width;
    view_camera->height = window_height;

    text_rect->x = window_width - offset.x;
}

void view_scroll_bar(filr_context *context, size_t ix, Rectangle view_camera) {
    float height = ((float) ix) / (float) context->size;
    DrawRectangle(view_camera.x + view_camera.width - 10, 0, 10, (int) (height * view_camera.height), GREEN);
}
