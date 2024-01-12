#include "lib/raylib.h"

//#define RAYGUI_IMPLEMENTATION
//#include "lib/raygui.h"

#include "filr.h"
#include "view.h"

#define FONT_DIR "resources/firacode.ttf"

#define INIT_WINDOW_HEIGHT 800
#define INIT_WINDOW_WIDTH 800


void handle_key_presses(filr_context *context, Rectangle *view_camera, Vector2 text_rect, Vector2 offset) {
    if (IsKeyPressed(KEY_DOWN)) {
        size_t ix = context->file_index;
        if (ix < context->size) ix++;
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


int main(void) {

    filr_context context = filr_init_context();

    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);

    InitWindow(INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT, "chuj");

    Font font = LoadFontEx(FONT_DIR, 32, 0, 250);
    Rectangle view_camera = {0, 0, INIT_WINDOW_WIDTH-30, INIT_WINDOW_HEIGHT};
    Vector2 offset = {30, 10};
    Vector2 text_rect = {INIT_WINDOW_WIDTH-30, 30};

    int previous_width = INIT_WINDOW_WIDTH;
    int previous_height = INIT_WINDOW_HEIGHT;

    while (!WindowShouldClose()) {
        int window_width = GetScreenWidth();
        int window_height = GetScreenHeight();

        bool did_resize = window_width != previous_width || window_height != previous_height;
        //if resize resze camera

        handle_key_presses(&context, &view_camera, text_rect, offset);

        BeginDrawing();
            ClearBackground(BLACK);

 /*           GuiScrollPanel(directory_bounds_rect,
                           NULL,
                           directory_content_rect,
                           &scroll,
                           &directory_view_rect);
        Rectangle directory_bounds_rect = {20, 40, 200, 150};
        Rectangle directory_content_rect = {0, 0, 180, 340};
        Rectangle directory_view_rect = {0};
        Vector2 scroll = {0, 0};

            BeginScissorMode(directory_view_rect.x, directory_view_rect.y, directory_view_rect.width, directory_view_rect.height);
                GuiGrid((Rectangle){directory_bounds_rect.x + scroll.x, 
                                    directory_bounds_rect.y + scroll.y, 
                                    directory_content_rect.width, 
                                    directory_content_rect.height}, 
                                    NULL, 
                                    16, 
                                    3, 
                                    NULL);
            EndScissorMode();
*/

            view_directory_contents(&context, view_camera, text_rect, offset, &font);
        EndDrawing();
    }

    filr_free_context(&context);

    CloseWindow();
    return 0;
}
