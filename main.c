#include "lib/raylib.h"
#include "filr.h"

void draw_directory_contents(const filr_file_array* array, int WINDOW_WIDTH, int WINDOW_HEIGHT) {
    for (size_t i = 0; i < array->size; ++i) {
        if (30 * (i + 1) + 10 > WINDOW_HEIGHT) return;
        DrawText(array->items[i].name, 20, 30 * i + 10, 20, RAYWHITE);
    }
}


int main(void) {
    char *HOME = getenv("HOMEPATH");

    filr_file_array array = filr_init_array();

    parse_directory_contents(HOME, &array);

    InitWindow(800, 1000, "chuj");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        //DrawText(array.items[array.size - 1].name, 400, 200, 20, RAYWHITE);
        draw_directory_contents(&array, 800, 1000);
        EndDrawing();
    }

    filr_free_array(&array);

    CloseWindow();
    return 0;
}
