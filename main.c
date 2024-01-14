#include "lib/raylib.h"

#include "filr.h"
#include "view.h"

#include <math.h>

#define INIT_WINDOW_HEIGHT 800
#define INIT_WINDOW_WIDTH 800

#define SCROLL_SPEED_CAP 1.0
#define SCROLL_STEP 0.5


void handle_key_presses(filr_context *context, view_t *view, int mouse_ix, float *scroll_pos_sum) {

    float mouse_wheel_move = 0;

    if (IsKeyPressed(KEY_Z)) {
        view_center_camera(context, view);
    }
    if (IsKeyPressed(KEY_DOWN)) {
        filr_move_index(context, 1);
        view_move_camera(context, view);
    }
    if (IsKeyPressed(KEY_UP)) {
        filr_move_index(context, -1);
        view_move_camera(context, view);
    }
    if (IsKeyPressed(KEY_ENTER)) {
        filr_goto_directory(context);
        filr_reset_index(context);
        view_move_camera(context, view);
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (context->file_index == mouse_ix) {
            filr_goto_directory(context);
            filr_reset_index(context);
            view_move_camera(context, view);
        } else if (mouse_ix != -1) context->file_index = mouse_ix;
    }
    if ((mouse_wheel_move = GetMouseWheelMove())) {
        *scroll_pos_sum += (mouse_wheel_move > SCROLL_SPEED_CAP)? SCROLL_SPEED_CAP : mouse_wheel_move;
        if (fabs(*scroll_pos_sum) > SCROLL_STEP) {
            int step = (int) (*scroll_pos_sum / SCROLL_STEP);
            *scroll_pos_sum -= (float) (step) * SCROLL_STEP;
            filr_move_index(context, -step);
            view_move_camera(context, view);
        }
    }
}


int main(void) {

    filr_context context = filr_init_context();

    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);

    InitWindow(INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT, "chuj");

    view_t view = view_init(INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT);


    int previous_width = INIT_WINDOW_WIDTH;
    int previous_height = INIT_WINDOW_HEIGHT;

    float scroll_pos_sum = 0;

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

            int mouse_ix = view_directory_contents(&context, view);
        EndDrawing();
        
        handle_key_presses(&context, &view, mouse_ix, &scroll_pos_sum);
    }

    filr_free_context(&context);
    view_free(view);

    CloseWindow();
    return 0;
}
