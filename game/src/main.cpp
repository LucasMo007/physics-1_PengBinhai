#include "raylib.h"
#include "raymath.h"

int main()
{
    InitWindow(800, 800, "Physics-1");

    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(WHITE);
            DrawText("Hello world!", 10, 10, 20, LIGHTGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
