
//#include "raylib.h"
//#include "raymath.h"
//#include <vector>
//
//struct PhysicsBody
//{
//    int id = -1;
//    Vector2 position = Vector2Zeros;
//    Vector2 velocity = Vector2Zeros;
//};
//
//struct PhysicsWorld
//{
//    Vector2 gravity = { 0.0f, 9.81f };
//    std::vector<PhysicsBody> entities;
//};
//
//constexpr float PROJECTILE_RADIUS = 10.0f;
//constexpr float LAUNCH_HEIGHT = 600.0f;
//
//int main()
//{
//    // An example of using C file libraries + raylib (which also uses C files internally)!
//    char* testData = new char[2048];
//    int byteCount = 0;
//    for (int i = 0; i < 3; i++)
//    {
//        int testNumber = i + 1;
//        float testRange = 100.0f;
//        float testTime = 500.0f;
//        byteCount += sprintf(testData + byteCount, "Range %i: %f units, Time %i: %f seconds.\n", testNumber, testRange, testNumber, testTime);
//    }
//
//    const char* fileName = "test.txt";
//    SaveFileText(fileName, testData);               // <-- Creates a new empty file (deletes previous file if there was one with the same name)
//    delete[] testData;
//
//    char* fileData = LoadFileText(fileName);        // <-- Loads a file as text data. File must exist otherwise an error is output to the console!
//    TraceLog(LOG_INFO, "File data: %s", fileData);  // Output our test (C-style) string to the console!
//
//    PhysicsWorld world;
//
//    for (int i = 0; i < 3; i++)
//    {
//        float launchAngle = (i + 1) * 10.0f * DEG2RAD;
//        float launchSpeed = 100.0f;
//
//        PhysicsBody entity{};
//        entity.id = i;
//        entity.position = { 0.0f, LAUNCH_HEIGHT - 1.0f };
//        entity.velocity = Vector2Rotate(Vector2UnitX, -launchAngle) * launchSpeed;
//        world.entities.push_back(entity);
//    }
//
//    InitWindow(800, 800, "Physics-1");
//    SetTargetFPS(60);
//    while (!WindowShouldClose())
//    {
//        float dt = GetFrameTime();
//
//        // Motion loop
//        for (size_t i = 0; i < world.entities.size(); i++)
//        {
//            PhysicsBody& e = world.entities[i];
//            Vector2 acc = world.gravity;
//
//            e.velocity += acc * dt;             // v = a * t
//            e.position += e.velocity * dt;      // p = v * t
//        }
//
//        // Collision loop
//        world.entities.erase(std::remove_if(world.entities.begin(), world.entities.end(),
//            [](PhysicsBody& entity)
//            {
//                bool remove = entity.position.y >= LAUNCH_HEIGHT;
//                if (remove)
//                {
//                    TraceLog(LOG_INFO, "Entity %i has hit the ground at %f.\n", entity.id, entity.position.x);
//                    // entity.position.x is the projectile's range
//                    // TODO -- Figure out how to keep track of each projectile's flight time!
//                    // TODO -- Figure out how to export all desired data to a .txt or .csv file!
//                }
//                return remove;
//            }
//        ), world.entities.end());
//
//        // Render loop
//        BeginDrawing();
//        ClearBackground(WHITE);
//        for (const PhysicsBody& e : world.entities)
//        {
//            DrawCircleV(e.position, PROJECTILE_RADIUS, LIGHTGRAY);
//        }
//        DrawRectangle(0, LAUNCH_HEIGHT, GetScreenWidth(), 20, DARKGRAY);
//        EndDrawing();
//    }
//
//    CloseWindow();
//    return 0;
//}
#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <cassert>

enum ColliderType
{
    COLLIDER_TYPE_INVALID,
    COLLIDER_TYPE_CIRCLE,
    COLLIDER_TYPE_HALF_SPACE
    //COLLDER_BOX <-- not implemented yet
};

union Collider
{
    struct
    {
        float radius;
    } circle;

    struct
    {
        Vector2 normal;
    } half_space;

    struct
    {
        Vector2 extents;
    } box;
};

struct PhysicsBody
{
    Vector2 position = Vector2Zeros;
    Vector2 velocity = Vector2Zeros;

    float drag = 1.0f;
    float inv_mass = 1.0f;
    float gravity_scale = 1.0f;

    ColliderType collider_type = COLLIDER_TYPE_INVALID;
    Collider collider{};
    bool collision = false;
    //Color color = MAGENTA;// Now colored based on collision status
};

// Physics Simulation
struct PhysicsWorld
{
    Vector2 gravity = { 0.0f, 9.81f };
    std::vector<PhysicsBody> entities;
};

// LE3 TODO -- Complete this function!
bool CircleCircle(Vector2 pos1, float rad1, Vector2 pos2, float rad2)
{
    return false;
}

// LE4 TODO -- Complete this function!
bool CircleHalfSpace(Vector2 pos_circle, float rad, Vector2 pos_half_space, Vector2 normal)
{
    // 1) Determine vector FROM half-space TO circle (AB = B - A)
    // 2) Determine the distance from circle to half-space by scalar-projecting AB onto normal
    // 3) Collision if distance is less than or equal to the radius of the circle
    return false;
}

int main()
{
    PhysicsWorld world;
    PhysicsBody* entity = nullptr;

    // Static circle
    world.entities.push_back({});
    entity = &world.entities.back();
    entity->position = { 400.0f, 400.0f };
    entity->gravity_scale = 0.0f;
    entity->collider_type = COLLIDER_TYPE_CIRCLE;
    entity->collider.circle.radius = 20.0f;

    // Static half-space
    world.entities.push_back({});
    entity = &world.entities.back();
    entity->position = { 400.0f, 600.0f };
    entity->gravity_scale = 0.0f;
    entity->collider_type = COLLIDER_TYPE_HALF_SPACE;
    entity->collider.half_space.normal = Vector2UnitY * -1.0f;
    //entity->collider.half_space.normal = Vector2Rotate(Vector2UnitX, -45.0f * DEG2RAD);
    // Simply rotate the normal if you'd like it to change directions!

    // Dynamic circle
    world.entities.push_back({});
    entity = &world.entities.back();
    entity->collider_type = COLLIDER_TYPE_CIRCLE;
    entity->collider.circle.radius = 20.0f;

    InitWindow(800, 800, "Physics-1");
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        entity->position = GetMousePosition();

        // Motion loop
        for (size_t i = 0; i < world.entities.size(); i++)
        {
            PhysicsBody& e = world.entities[i];
            Vector2 acc = world.gravity * e.gravity_scale;

            e.velocity += acc * dt;             // v = a * t
            e.position += e.velocity * dt;      // p = v * t

            // Reset collision render status
            e.collision = false;
        }

        // Collision loop (Test every object against all other objects)
        for (size_t i = 0; i < world.entities.size(); i++)
        {
            for (size_t j = i + 1; j < world.entities.size(); j++)
            {
                PhysicsBody& a = world.entities[i];
                PhysicsBody& b = world.entities[j];
                assert(a.collider_type != COLLIDER_TYPE_INVALID && b.collider_type != COLLIDER_TYPE_INVALID);
                bool collision = false;

                if (a.collider_type == COLLIDER_TYPE_CIRCLE &&
                    b.collider_type == COLLIDER_TYPE_CIRCLE)
                {
                    collision = CircleCircle(
                        a.position, a.collider.circle.radius,
                        b.position, b.collider.circle.radius
                    );
                }
                else if (
                    a.collider_type == COLLIDER_TYPE_CIRCLE &&
                    b.collider_type == COLLIDER_TYPE_HALF_SPACE)
                {
                    collision = CircleHalfSpace(
                        a.position, a.collider.circle.radius,
                        b.position, b.collider.half_space.normal
                    );
                }
                else if (
                    a.collider_type == COLLIDER_TYPE_HALF_SPACE &&
                    b.collider_type == COLLIDER_TYPE_CIRCLE)
                {
                    collision = CircleHalfSpace(
                        b.position, b.collider.circle.radius,
                        a.position, a.collider.half_space.normal
                    );
                }

                a.collision |= collision;
                b.collision |= collision;
            }
        }

        BeginDrawing();
        ClearBackground(WHITE);
        for (const PhysicsBody& e : world.entities)
        {
            Color color = e.collision ? RED : GREEN;
            if (e.collider_type == COLLIDER_TYPE_CIRCLE)
            {
                DrawCircleV(e.position, e.collider.circle.radius, color);
            }
            else if (e.collider_type == COLLIDER_TYPE_HALF_SPACE)
            {
                // "Flip" the normal to determine the direction of the half-space
                Vector2 direction = { -e.collider.half_space.normal.y, e.collider.half_space.normal.x };
                Vector2 p0 = e.position + direction * 1000.0f;
                Vector2 p1 = e.position - direction * 1000.0f;
                DrawLineEx(p0, p1, 5.0f, color);
                DrawLineEx(e.position, e.position + e.collider.half_space.normal * 50.0f, 5.0f, GOLD);
            }
        }

        // Hard-coded visuals for LE4:
        Vector2 circle_pos = world.entities[2].position;
        Vector2 half_space_pos = world.entities[1].position;
        Vector2 normal = world.entities[1].collider.half_space.normal;

        // AB = B - A
        Vector2 to_circle = circle_pos - half_space_pos;
        float proj = Vector2DotProduct(to_circle, normal);
        DrawLineEx(half_space_pos, half_space_pos + to_circle, 5.0f, MAGENTA);
        DrawCircleV(half_space_pos + normal * proj, 10.0f, PINK);
        DrawLineEx(half_space_pos, half_space_pos + normal * proj, 4.0f, PINK);
        // if proj is less than the radius of the circle, there's collision!

        EndDrawing();
    }

    CloseWindow();
    return 0;
}