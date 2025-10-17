#include "raylib.h"
#include "raymath.h"
#include <cmath>
#include <algorithm> 
#include <cstdio>
#include <vector>  
//1.	Develop a framework using multiple Physics Body objects, which contain data for:a.position b.velocity c.drag d.	mass 

struct PhysicsBody {
    Vector2 position = Vector2Zeros;
    Vector2 velocity = Vector2Zeros;
    float   drag = 1.0f;
    float   mass = 1.0f;
    bool    active = true;
};
//a Physics Simulation class 
class PhysicsWorld {
public:
    float   time = 0.0f;
    Vector2 gravity{ 0.0f, 600.0f };
    void Add(PhysicsBody* b);
    void Step(float dt);
private:
    std::vector<PhysicsBody*> bodies;
};
void PhysicsWorld::Add(PhysicsBody* b) {
    bodies.push_back(b);
}

void PhysicsWorld::Step(float dt) {
    time += dt;
    for (auto* b : bodies) {
        if (!b || !b->active) continue;

        b->velocity.x += gravity.x * dt;
        b->velocity.y += gravity.y * dt;
        b->position.x += b->velocity.x * dt;
        b->position.y += b->velocity.y * dt;
    }
}
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

    float   launchAngleDeg = 45.0f;
    float   launchSpeed = 300.0f;
    float   lengthPerSpeed = 0.5f;
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();//获取帧时间增量：获取上一帧到当前帧的时间间隔（秒），用于实现与帧率无关的平滑移动

        bool  fast = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
        float angleStep = (fast ? 90.0f : 45.0f) * dt;
        float speedStep = (fast ? 400.0f : 200.0f) * dt;
        float posStep = (fast ? 300.0f : 150.0f) * dt;

        if (IsKeyDown(KEY_LEFT))  launchAngleDeg -= angleStep;
        if (IsKeyDown(KEY_RIGHT)) launchAngleDeg += angleStep;
        if (IsKeyDown(KEY_UP))    launchSpeed += speedStep;
        if (IsKeyDown(KEY_DOWN))  launchSpeed = std::max(0.0f, launchSpeed - speedStep);

        if (IsKeyDown(KEY_A)) launchPosition.x -= posStep;
        if (IsKeyDown(KEY_D)) launchPosition.x += posStep;
        if (IsKeyDown(KEY_W)) launchPosition.y -= posStep;
        if (IsKeyDown(KEY_S)) launchPosition.y += posStep;

        launchAngleDeg = Clamp(launchAngleDeg, -89.0f, 89.0f);

        float rad = launchAngleDeg * DEG2RAD;
        float vx = launchSpeed * cosf(rad);
        float vy = launchSpeed * sinf(rad);
        Vector2 v0{ vx, vy };

        Vector2 dir = v0;
        float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
        if (len > 0.0f) { dir.x /= len; dir.y /= len; }


        Vector2 start = launchPosition;
        Vector2 end{
            start.x + dir.x * (launchSpeed * lengthPerSpeed),
            start.y - dir.y * (launchSpeed * lengthPerSpeed)
        };
        BeginDrawing();
        ClearBackground(RAYWHITE);


        DrawRectangleRec(platform, GRAY);


        DrawRectangleRec(ground, DARKGRAY);


        DrawCircleV(launchPosition, radius, RED);

        DrawCircleV(start, 6.0f, MAROON);
        DrawLineEx(start, end, 3.0f, RED);

        DrawRectangle(10, 10, 500, 120, Fade(BLACK, 0.06f));
        DrawRectangleLines(10, 10, 500, 120, Fade(BLACK, 0.2f));
        char buf[256];
        std::snprintf(buf, sizeof(buf),
            "launchPosition: (%.1f, %.1f)\nlaunchAngle: %.1f deg\nlaunchSpeed: %.1f\nv0 (y-up): (%.1f, %.1f)",
            start.x, start.y, launchAngleDeg, launchSpeed, vx, vy);
        DrawText(buf, 18, 18, 18, BLACK);

        DrawText("Move: A/D/W/S   | Arrow Left/Right:angle | Arrow Up/Down:speed ",
            18, 135, 18, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
