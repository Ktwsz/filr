#include "lib/raylib.h"
#include "filr.h"

void draw_directory_contents(const filr_context context, int WINDOW_WIDTH, int WINDOW_HEIGHT) {
    for (size_t i = 0; i < context.size; ++i) {
        if (30 * (i + 1) + 10 > WINDOW_HEIGHT) return;
        Color highlight_color = RAYWHITE;
        if (context.file_index == i) highlight_color = GREEN;
        DrawText(filr_get_file_name(&context, i), 20, 30 * i + 10, 20, highlight_color);
    }
}


void handle_key_presses(filr_context *context) {
    if (IsKeyReleased(KEY_DOWN)) {
        size_t ix = context->file_index;
        if (ix < context->size) ix++;
        filr_move_index(context, ix);
    }
    if (IsKeyReleased(KEY_UP)) {
        size_t ix = context->file_index;
        if (ix > 0) ix--;
        filr_move_index(context, ix);
    }
    if (IsKeyReleased(KEY_ENTER)) {
        filr_goto_directory(context);
        filr_reset_index(context);
    }
}


int main(void) {

    filr_context context = filr_init_context();

    InitWindow(800, 1000, "chuj");

    while (!WindowShouldClose()) {
        handle_key_presses(&context);
        BeginDrawing();
        ClearBackground(BLACK);
        draw_directory_contents(context, 800, 1000);
        EndDrawing();
    }

    filr_free_context(&context);

    CloseWindow();
    return 0;
}
