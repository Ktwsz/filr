#include "raylib.h"

#include "filr.h"
#include "view.h"
#include "inputs.h"
#include <assert.h>

#define INIT_WINDOW_HEIGHT 800
#define INIT_WINDOW_WIDTH 1000


int main(void) {
    filr_context context_array[2] = {0};

    result err_load1, err_load2;

    err_load1 = filr_init_context(&context_array[0]);
    assert(!err_load1.err);

    err_load2 = filr_init_context(&context_array[1]);
    assert(!err_load2.err);


    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);

    InitWindow(INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT, "filr");
    SetWindowMinSize(INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT);

    SetTargetFPS(30);

    result err_load_view;
    view_t view;
    err_load_view = view_init(&view, (float)INIT_WINDOW_WIDTH, (float)INIT_WINDOW_HEIGHT);
    assert(!err_load_view.err);

    inputs_t input = {0};
    inputs_init(&input, &view);

    while (!WindowShouldClose()) {
        view_handle_resize(context_array, &view, input.window_focus);

        BeginDrawing();
            view_draw_background(&view);

            view_view(context_array, &view, &input, mouse_input_callback, input.window_focus);
        EndDrawing();

        handle_key_presses(context_array, &view, &input);
    }

    filr_free_context(&context_array[0]);
    if (view.second_window.show)
        filr_free_context(&context_array[1]);
    view_free(&view);

    CloseWindow();
    return 0;
}
