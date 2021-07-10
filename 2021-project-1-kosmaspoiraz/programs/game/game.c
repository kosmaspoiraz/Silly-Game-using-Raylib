#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"

#include "interface.h"
#include "state.h"

State state;
KeyState keys;

static void update_and_draw()
{
    // Πληροφορίες για τα πλήκτρα
    keys->enter = IsKeyPressed(KEY_ENTER);
    keys->left = IsKeyDown(KEY_LEFT);
    keys->n = IsKeyPressed(KEY_N);
    keys->p = IsKeyPressed(KEY_P);
    keys->right = IsKeyDown(KEY_RIGHT);
    keys->up = IsKeyDown(KEY_UP);

    state_update(state, keys);
    interface_draw_frame(state);
}

int main()
{
    state = state_create();
    keys = malloc(sizeof(KeyState));

    interface_init();

    start_main_loop(update_and_draw);

    state_destroy(state);

    interface_close();

    return 0;
}