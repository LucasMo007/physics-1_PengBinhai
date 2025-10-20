#include "raylib.h"
#include "raymath.h"
#include <cmath>
#include <algorithm> 
#include <cstdio>
#include <vector>  
//1.	Develop a framework using multiple Physics Body objects, which contain data for:a.position b.velocity c.drag d.	mass 
enum class ShapeType { None, Sphere, Halfspace};

struct PhysicsBody 
{
    Vector2 position = Vector2Zeros;
    Vector2 velocity = Vector2Zeros;
    float   drag = 1.0f;
    float   mass = 1.0f;
    bool    active = true;
    // --- Collision shape ---
    ShapeType shape = ShapeType::None;
    float radius = 0.0f;     // used when shape == Sphere

    // --- Debug/visual ---
    bool  isOverlapping = false;   // set by collision system
    Color baseColor = DARKGRAY;    // draw base; turn RED if overlapping

    Vector2 normal{ 0, -1 };  // bird get into halfspace inside ,it happen collision。

                             /* 地面以下 是halfspace（inside），
                                normal(0, −1) means up part of line is  halfspace outside，
                               */
    /*在游戏设计中，法向量(normal) 指向半空间的外部。

        要发生碰撞，物体必须进入半空间内部（即法线的反方向那一侧）。*/
    float   offset = 0.0f;

};

struct Trail 
{
    std::vector<Vector2> points;
    void Clear() { points.clear(); }
    void Add(Vector2 p) {
        if (points.empty() || Vector2Distance(points.back(), p) > 2.0f)
            points.push_back(p);
    }
    void Draw(Color c) const {
        for (size_t i = 1; i < points.size(); ++i) {
            DrawLineEx(points[i - 1], points[i], 2.0f, c);
        }
    }
};
//a Physics Simulation class 
class PhysicsWorld 
{
public:
    float   time = 0.0f;
    Vector2 gravity{ 0.0f, 600.0f };
    void Add(PhysicsBody* b);
    void Step(float dt);
private:
    std::vector<PhysicsBody*> bodies;
    void CollideAll();  // NEW: pairwise collision checks (sphere-sphere)
};
void PhysicsWorld::Add(PhysicsBody* b)
{
    bodies.push_back(b);
}

void PhysicsWorld::Step(float dt) 
{
    time += dt;
    for (auto* b : bodies) {
        if (!b || !b->active) continue;

        b->velocity.x += gravity.x * dt;
        b->velocity.y += gravity.y * dt;
        b->position.x += b->velocity.x * dt;
        b->position.y += b->velocity.y * dt;
    }
    CollideAll();
}
void PhysicsWorld::CollideAll() {
    // reset flags
    for (auto* b : bodies) if (b) b->isOverlapping = false;

    // pairwise checks
    const int n = (int)bodies.size();
    for (int i = 0; i < n; ++i) {
        PhysicsBody* a = bodies[i];
        if (!a) continue;

        for (int j = i + 1; j < n; ++j) {
            PhysicsBody* c = bodies[j];
            if (!c) continue;

            // Sphere-Sphere only
            if (a->shape == ShapeType::Sphere && c->shape == ShapeType::Sphere) {
                float sumR = a->radius + c->radius;
                float dist = Vector2Distance(a->position, c->position);
                if (dist < sumR) {
                    a->isOverlapping = true;
                    c->isOverlapping = true;
                }
            }
        }
    }
    // --- B) 球-半空间（半空间为 n·x >= offset 的一侧）---
    for (int i = 0; i < n; ++i) {
        PhysicsBody* s = bodies[i];
        if (!s || s->shape != ShapeType::Sphere) continue;

        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            PhysicsBody* h = bodies[j];
            if (!h || h->shape != ShapeType::Halfspace) continue;

            Vector2 nrm = h->normal;
            float len = Vector2Length(nrm);
            if (len <= 0.0f) continue;                 // 避免无效法线
            nrm = Vector2Scale(nrm, 1.0f / len);       // 归一化

            // 有符号距离：d = n·c - offset
            float d = Vector2DotProduct(nrm, s->position) - h->offset;

           
            if (d <= s->radius) 
            {
                s->isOverlapping = true;
                h->isOverlapping = true;
            }
        }
    }
}
int main()
{
    static constexpr int InitialWidth = 1200;
    static constexpr int InitialHeight = 800;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(800, 800, "Angry Bird: Lab 4");
    SetTargetFPS(60);



    Rectangle platform;
    platform.x = 0.0f;
    platform.y = 600.0f;
    platform.width = 100.0f;
    platform.height = 20.0f;

    const float gapBelowPlatform = 36.0f;
    float greenLineY = platform.y + platform.height + gapBelowPlatform;


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

    //gavvity adjust avriable 
    float gravityAngleDeg = 90.0f;
    float gravityMag = 700.0f;
    //one object of PhysicsWorld 
    PhysicsWorld sim;

    PhysicsBody bird;
    bird.position = launchPosition;
    bird.velocity = { 0,0 };
    bird.active = false;
    bird.shape = ShapeType::Sphere;
    bird.radius = 10.0f;
    bird.baseColor = ORANGE;
    sim.Add(&bird);

    PhysicsBody target;
    target.position = { 500.0f, 520.0f }; // adjust as you like
    target.velocity = { 0,0 };
    target.active = false;              // static (not integrated)
    target.shape = ShapeType::Sphere;
    target.radius = 24.0f;
    target.baseColor = DARKBLUE;           // will turn RED on overlap
    sim.Add(&target);

    Trail trail;

    auto Launch = [&]()
        {
            bird.active = true;
            bird.position = launchPosition;

            float rad = launchAngleDeg * DEG2RAD;
            bird.velocity.x = launchSpeed * cosf(rad);
            bird.velocity.y = -launchSpeed * sinf(rad);
            trail.Clear();
            trail.Add(bird.position);
        };

    auto SetAnglePreset = [&](int key)
        {
            if (key == KEY_ONE)   launchAngleDeg = 0.0f;
            if (key == KEY_TWO)   launchAngleDeg = 45.0f;
            if (key == KEY_THREE) launchAngleDeg = 60.0f;
            if (key == KEY_FOUR)  launchAngleDeg = 90.0f;
        };
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();//获取帧时间增量：获取上一帧到当前帧的时间间隔（秒），用于实现与帧率无关的平滑移动

        if (IsKeyPressed(KEY_SPACE)) Launch();


        if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_TWO) ||
            IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_FOUR)) {
            SetAnglePreset(GetKeyPressed());
        }

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
        //4.Ensure the direction and magnitude of gravity can be adjusted. 
        float gAngStep = 120.0f * dt;
        float gMagStep = 600.0f * dt;
        if (IsKeyDown(KEY_J)) gravityAngleDeg -= gAngStep;
        if (IsKeyDown(KEY_L)) gravityAngleDeg += gAngStep;
        if (IsKeyDown(KEY_I)) gravityMag += gMagStep;
        if (IsKeyDown(KEY_K)) gravityMag = std::max(0.0f, gravityMag - gMagStep);

        float gRad = gravityAngleDeg * DEG2RAD;
        sim.gravity.x = gravityMag * cosf(gRad);
        sim.gravity.y = gravityMag * sinf(gRad);


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
            start.x + dir.x * (launchSpeed * lengthPerSpeed * 0.35f),
            start.y - dir.y * (launchSpeed * lengthPerSpeed * 0.35f)
        };

        if (bird.active) 
        {
            sim.Step(dt);
            trail.Add(bird.position);

            if (bird.position.y > ground.y - radius ||
                bird.position.x > GetScreenWidth() + 50 ||
                bird.position.x < -50)
            {
                bird.active = false;
            }
            else 
            {
                sim.Step(0.0f);
            }
        }

        bool hitGreenOrBelow = (bird.position.y + bird.radius >= greenLineY);
        float rr = bird.radius + target.radius;
        bool hitTarget = (Vector2Distance(bird.position, target.position) < rr);
            BeginDrawing();
            ClearBackground(RAYWHITE);


            DrawRectangleRec(platform, GRAY);


            DrawRectangleRec(ground, DARKGRAY);

            trail.Draw(ORANGE);


            Color birdColor = hitTarget ? PURPLE : (hitGreenOrBelow ? RED : bird.baseColor);
            DrawCircleV(bird.position, bird.radius, birdColor);
            DrawCircleV(target.position, target.radius, target.isOverlapping ? RED : target.baseColor);

            DrawCircleV(start, 6.0f, MAROON);
            DrawLineEx(start, end, 3.0f, RED);

            Color areaFill = hitGreenOrBelow ? Color{ 255, 0, 0, 110 } : Color{ 0, 200, 0, 70 };
            DrawRectangle(0, (int)greenLineY, GetScreenWidth(),
                GetScreenHeight() - (int)greenLineY, areaFill);

            DrawLineEx({ 0, greenLineY }, { (float)GetScreenWidth(), greenLineY }, 5.0f, GREEN);


            {
                Vector2 gStart{ 350, 40 };
                Vector2 gEnd = Vector2Add(gStart, Vector2Scale(sim.gravity, 0.08f));
                DrawLineEx(gStart, gEnd, 4.0f, BLUE);
                DrawCircleV(gStart, 5.0f, DARKBLUE);
                DrawText("Gravity", (int)gStart.x - 50, (int)gStart.y - 18, 16, DARKBLUE);
            }


            DrawRectangle(10, 10, 500, 150, Fade(BLACK, 0.06f));
            DrawRectangleLines(10, 10, 500, 150, Fade(BLACK, 0.2f));
            char buf[512];
            std::snprintf(buf, sizeof(buf),
                "launchPosition: (%.1f, %.1f)\nlaunchAngle: %.1f deg\nlaunchSpeed: %.1f\nv0 (y-up): (%.1f, %.1f)\nGravity: mag=%.1f  ang=%.1f deg (0=right, 90=down)\nTime: %.2f s   Active: %s\nTouchGreen: % s   HitTarget : % s\n",
                start.x, start.y, launchAngleDeg, launchSpeed, vx, vy, gravityMag, gravityAngleDeg, sim.time, bird.active ? "yes" : "no",hitGreenOrBelow ? "YES" : "no",
                hitTarget ? "YES" : "no" );


            DrawText(buf, 22, 18, 18, BLACK);

            DrawText("Move: A/D/W/S   | Arrow Left/Right:angle | Arrow Up/Down:speed ",
                18, 170, 18, DARKGRAY);
            DrawText("SPACE=Launch | 1/2/3/4 => 0/45/60/90 deg", 16, 190, 18, DARKGRAY);
            DrawText("J/L=gravity dir  I/K=gravity mag", 16, 212, 18, DARKGRAY);

            EndDrawing();
        }

        CloseWindow();
        return 0;
    }

