#include <stdlib.h>

#include "interface.h"
#include "raylib.h"

#include "state.h"

#define CHAR_X SCREEN_WIDTH / 2
#define CHAR_Y SCREEN_HEIGHT / 3

// Assets
// Texture char_img;
Texture megaman;
Texture rev_megaman;
Texture llama;
Texture rev_llama;
Image *img_llama;
Image *img_megaman;
Texture door;
Texture obstacle;

// Αρχικοποιεί το interface του παιχνιδιού
void interface_init()
{
    // Αρχικοποίηση του παραθύρου
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "game");
    SetTargetFPS(60);

    // Φόρτωση εικόνων και ήχων
    // char_img = LoadTextureFromImage(LoadImage("assets/megaman.png"));
    megaman = LoadTextureFromImage(LoadImage("assets/megaman.png"));
    img_megaman = malloc(sizeof(Image));
    *img_megaman = LoadImage("assets/megaman.png");
    ImageFlipHorizontal(img_megaman);
    rev_megaman = LoadTextureFromImage(*img_megaman);

    llama = LoadTextureFromImage(LoadImage("assets/llama.png"));
    img_llama = malloc(sizeof(Image));
    *img_llama = LoadImage("assets/llama.png");
    ImageFlipHorizontal(img_llama);
    rev_llama = LoadTextureFromImage(*img_llama);

    door = LoadTextureFromImage(LoadImage("assets/door.png"));
    obstacle = LoadTextureFromImage(LoadImage("assets/obstacle.png"));
}

// Κλείνει το interface του παιχνιδιού
void interface_close()
{
    CloseWindow();
}

// Σχεδιάζει ένα frame με την τωρινή κατάσταση του παιχνδιού
void interface_draw_frame(State state)
{
    StateInfo info = state_info(state);
    List objects = malloc(sizeof(List));
    objects = state_objects(state, info->character->rect.x - SCREEN_WIDTH, info->character->rect.x + SCREEN_WIDTH);

    BeginDrawing();

    // Καθαρισμός, θα τα σχεδιάσουμε όλα από την αρχή
    ClearBackground(RAYWHITE);

    // Σχεδιάζουμε τον χαρακτήρα και τα αντικείμενα
    DrawLine(-SCREEN_WIDTH, info->character->rect.height + CHAR_Y / 0.54 - 9, SCREEN_WIDTH, info->character->rect.height + CHAR_Y / 0.54 - 9, BLACK);
    // DrawTexture(char_img, CHAR_X, info->character->rect.y - CHAR_Y, BLACK);
    if (info->character->forward)
    {
        DrawTexture(megaman, CHAR_X, info->character->rect.y - CHAR_Y + 10, WHITE);
    }
    else
    {
        DrawTexture(rev_megaman, CHAR_X, info->character->rect.y - CHAR_Y + 10, WHITE);
    }

    if (objects != NULL)
    {
        for (ListNode obj_node = list_first(objects); obj_node != LIST_EOF; obj_node = list_next(objects, obj_node))
        {
            Object obj = list_node_value(objects, obj_node);
            if (obj->type == ENEMY)
            {
                // DrawRectangle(obj->rect.x - info->character->rect.x + CHAR_X, obj->rect.y - CHAR_Y, obj->rect.width, obj->rect.height, BLUE);
                // DrawCircle(obj->rect.x - info->character->rect.x + CHAR_X, obj->rect.y - CHAR_Y, 20, BLUE);
                if (obj->forward)
                {
                    DrawTexture(llama, obj->rect.x - info->character->rect.x + CHAR_X, obj->rect.y - CHAR_Y - 10, WHITE);
                }
                else
                {
                    DrawTexture(rev_llama, obj->rect.x - info->character->rect.x + CHAR_X, obj->rect.y - CHAR_Y - 10, WHITE);
                }
            }
            else if (obj->type == OBSTACLE)
            {
                // DrawRectangle(obj->rect.x - info->character->rect.x + CHAR_X, obj->rect.y - CHAR_Y, obj->rect.width, obj->rect.height, RED);
                DrawTexture(obstacle, obj->rect.x - info->character->rect.x + CHAR_X, obj->rect.y - CHAR_Y + 50, WHITE);
            }
            else if (obj->type == PORTAL)
            {
                // DrawRectangleLines(obj->rect.x - info->character->rect.x + CHAR_X, obj->rect.y - CHAR_Y, obj->rect.width, obj->rect.height, GREEN);
                DrawTexture(door, obj->rect.x - info->character->rect.x + CHAR_X, obj->rect.y - CHAR_Y - 25, WHITE);
            }
        }
    }

    // Σχεδιάζουμε το σκορ και το FPS counter
    DrawText(TextFormat("%04i", info->wins), 20, 20, 40, GRAY);
    DrawText(TextFormat("%04i", info->current_portal), 20, 60, 40, GRAY);
    DrawFPS(SCREEN_WIDTH - 80, 0);

    // Αν το παιχνίδι έχει τελειώσει, σχεδιάζομαι το μήνυμα για να ξαναρχίσει
    if (info->playing == false)
    {
        DrawText(
            "PRESS [ENTER] TO PLAY AGAIN",
            GetScreenWidth() / 2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20) / 2,
            GetScreenHeight() / 2 - 50, 20, GRAY);
    }

    // Αν το παιχνίδι έχει γίνει paused, σχεδιάζομαι το μήνυμα για να ξαναρχίσει
    if (info->paused == true)
    {
        DrawText(
            "PRESS [N] TO RESUME FOR 1 FRAME",
            GetScreenWidth() / 2 - MeasureText("PRESS [Ν] TO RESUME FOR 1 FRAME", 20) / 2,
            GetScreenHeight() / 2 - 50, 20, GRAY);
    }

    // list_destroy(objects);

    EndDrawing();
}
