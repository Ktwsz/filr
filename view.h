#ifndef VIEW_H
#define VIEW_H

#include "lib/raylib.h"
#include "filr.h"

void view_directory_contents(filr_context *context, Rectangle view_camera, Vector2 text_rect, Vector2 offset, Font *font);

void view_center_camera(filr_context *context, Rectangle *view_camera, Vector2 text_rect, Vector2 offset);

void view_move_camera(filr_context *context, Rectangle *view_camera, Vector2 text_rect, Vector2 offset);

void view_scroll_bar(filr_context *context, Rectangle *view_camera);

#endif
