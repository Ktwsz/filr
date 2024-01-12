#include "lib/raylib.h"

#include "filr.h"
#include "view.h"

#define FONT_DIR "resources/firacode.ttf"

#define INIT_WINDOW_HEIGHT 800
#define INIT_WINDOW_WIDTH 800


void handle_key_presses(filr_context *context, Rectangle *view_camera, Vector2 text_rect, Vector2 offset) {
    if (IsKeyDown(KEY_LEFT_CONTROL)) {
        if (IsKeyPressed(KEY_Y)) {
            view_center_camera(context, view_camera, text_rect, offset);
        }
    } else {
        if (IsKeyPressed(KEY_DOWN)) {
            size_t ix = context->file_index;
            if (ix < context->size - 1) ix++;
            filr_move_index(context, ix);
            view_move_camera(context, view_camera, text_rect, offset);
        }
        if (IsKeyPressed(KEY_UP)) {
            size_t ix = context->file_index;
            if (ix > 0) ix--;
            filr_move_index(context, ix);
            view_move_camera(context, view_camera, text_rect, offset);
        }
        if (IsKeyPressed(KEY_ENTER)) {
            filr_goto_directory(context);
            filr_reset_index(context);
            view_move_camera(context, view_camera, text_rect, offset);
        }
    }
}


int main(void) {

    filr_context context = filr_init_context();

    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);

    InitWindow(INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT, "chuj");

    Font font = LoadFontEx(FONT_DIR, 32, 0, 250);
    Rectangle view_camera = {.x = 0, .y = 0, .width = INIT_WINDOW_WIDTH, .height = INIT_WINDOW_HEIGHT};
    Vector2 offset = {.x = 30, .y = 10};
    Vector2 text_rect = {.x = INIT_WINDOW_WIDTH-30, .y = 30};

    int previous_width = INIT_WINDOW_WIDTH;
    int previous_height = INIT_WINDOW_HEIGHT;

    while (!WindowShouldClose()) {
        int window_width = GetScreenWidth();
        int window_height = GetScreenHeight();

        bool did_resize = window_width != previous_width || window_height != previous_height;
        if (did_resize) {
            view_resize_rects(&view_camera, &text_rect, offset, window_width, window_height);
            view_center_camera(&context, &view_camera, text_rect, offset);
            previous_width = window_width;
            previous_height = window_height;
        }

        handle_key_presses(&context, &view_camera, text_rect, offset);

        BeginDrawing();
            ClearBackground(BLACK);

            view_directory_contents(&context, view_camera, text_rect, offset, &font);
        EndDrawing();
    }

    filr_free_context(&context);

    CloseWindow();
    return 0;
}
