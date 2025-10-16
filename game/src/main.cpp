#include "raylib.h"
#include "raymath.h"

int main()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(800, 800, "Angry Bird: Rendering Launch Vector");
    SetTargetFPS(60);

    Rectangle platform;
    platform.x = 0.0f;
    platform.y = 600.0f;
    platform.width = 100.0f;
    platform.height = 20.0f;

    Rectangle ground;
    ground.x = 0.0f;
    ground.y = 780.0f;
    ground.width = 800.0f;
    ground.height = 20.0f;

    float radius = 10.0f;
    Vector2 launchPosition;
    launchPosition.x = platform.x + platform.width - radius;
    launchPosition.y = platform.y - radius;
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);


        DrawRectangleRec(platform, GRAY);


        DrawRectangleRec(ground, DARKGRAY);


        DrawCircleV(launchPosition, radius, RED);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
