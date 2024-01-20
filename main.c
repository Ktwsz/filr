#include "lib/raylib.h"

#include "filr.h"
#include "view.h"
#include "inputs.h"

#include <math.h>

#define INIT_WINDOW_HEIGHT 800
#define INIT_WINDOW_WIDTH 800


int main(void) {

    filr_cmp_array cmp_array;
    filr_init_cmp_array(&cmp_array);

    filr_context context = {0};
    filr_init_context(&context);
    qsort(&(context.files[2]), context.size - 2, sizeof(filr_file), cmp_array.array[cmp_array.ix]);

    inputs_t input = {0};
    inputs_init(&input);

    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);

    InitWindow(INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT, "chuj");

    SetTargetFPS(30);

    view_t view;
    view_init(&view, INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT);


    int previous_width = INIT_WINDOW_WIDTH;
    int previous_height = INIT_WINDOW_HEIGHT;

    while (!WindowShouldClose()) {
        int window_width = GetScreenWidth();
        int window_height = GetScreenHeight();

        bool did_resize = window_width != previous_width || window_height != previous_height;
        if (did_resize) {
            view_resize(&view, window_width, window_height);
            view_center_camera(&context, &view);
            previous_width = window_width;
            previous_height = window_height;
        }

        BeginDrawing();
            view_draw_background(view);

            input.mouse_ix = view_directory_contents(&context, view);
        EndDrawing();
        
        handle_key_presses(&context, &view, &input, &cmp_array);
    }

    filr_free_context(&context);
    view_free(view);

    CloseWindow();
    return 0;
}
