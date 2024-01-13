#ifndef VIEW_H
#define VIEW_H

#include "lib/raylib.h"
#include "filr.h"

int view_directory_contents(filr_context *context, Rectangle view_camera, Vector2 text_rect, Vector2 offset, Font *font);

void view_center_camera(filr_context *context, Rectangle *view_camera, Vector2 text_rect, Vector2 offset);

void view_move_camera(filr_context *context, Rectangle *view_camera, Vector2 text_rect, Vector2 offset);

void view_resize_rects(Rectangle *view_camera, Vector2 *text_rect, Vector2 offset, int window_width, int window_height);

void view_scroll_bar(filr_context *context, size_t ix, Rectangle view_camera);

#endif
