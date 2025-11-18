
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
    Vector2 net_force = Vector2Zeros;

    float drag = 1.0f;
    float inv_mass = 1.0f;
    float gravity_scale = 1.0f;

    float friction_coeff = 1.0f;        // How easily an object moves along a surface --> 0 = easy, 1 = hard
    float restitution_coeff = 1.0f;     // "Bounciness" --> how much energy is lost on-collision (0 = all energy lost, 1 = no energy lost)

    ColliderType collider_type = COLLIDER_TYPE_INVALID;
    Collider collider{};
    bool collision = false;
    //Color color = MAGENTA;// Now colored based on collision status
};
struct HitPair
{
    //int a = -1;
    //int b = -1;
    PhysicsBody* a = nullptr;
    PhysicsBody* b = nullptr;
    Vector2 mtv = Vector2Zeros;
};
// Physics Simulation
struct PhysicsWorld
{
    Vector2 gravity = { 0.0f, 9.81f };
    std::vector<PhysicsBody> entities;
};
// LE3 TODO -- Complete this function!
bool CircleCircle(Vector2 pos1, float rad1, Vector2 pos2, float rad2, Vector2* mtv = nullptr)
{
    float sumRadii = rad1 + rad2;
    float distance = Vector2Distance(pos1, pos2);
    bool collision = distance <= sumRadii;
    // LE5:
    if (collision && mtv != nullptr)
    {
        //  -mtv magnitude is distance between centres  - radii sum 
        float mag = sumRadii - distance;
        //  -mtv direction is a unit-vector FROM circle2 TO circle 1 (mtv resolves 1 FROM 2)
        Vector2 dir  = Vector2Normalize(pos1 - pos2);
        *mtv = dir * mag;
    }

    return collision  ;
}
// LE4 TODO -- Complete this function!
bool CircleHalfSpace(Vector2 pos_circle, float rad, Vector2 pos_half_space, Vector2 normal, Vector2* mtv = nullptr)
{
       Vector2 to_circle = pos_circle - pos_half_space;
   
        float proj = Vector2DotProduct(to_circle, normal);
 
        bool collision = proj <= rad;
        if (collision && mtv != nullptr)
        {
            //  -mtv magnitude is radius - projection
            float mag = rad - proj;
            //  -mtv direction is just plane normal!
            Vector2 dir = normal;

            *mtv = dir* mag;
            /* <-- remove this, just a test to see that the mtv is applied!*/
        }
        return collision;
}
void UpdateMotion(PhysicsWorld& world);
std::vector<HitPair> DetectCollisions(PhysicsWorld& world);
//void ResolveCollisions(PhysicsWorld& world, std::vector<HitPair> collisions);
void ValidateResolutionVectors(std::vector<HitPair>& collisions);
void ResolveCollisions(std::vector<HitPair> collisions);

void Update(PhysicsWorld& world);
void Draw(const PhysicsWorld& world);
void DrawProjCircleHalfSpace(Vector2 pos_circle, float rad, Vector2 pos_half_space, Vector2 normal);
// Inverse - Mass of 0.0 means "infinitely heavy" -- > 1.0f / 0.0f -- > "infinity" for our purposes
bool IsMassInfinite(const PhysicsBody & entity)
{
    return entity.inv_mass <= FLT_EPSILON;
}


int main()
{
    PhysicsWorld world;
    {
        PhysicsBody* entity = nullptr;

        Vector2 test_force = Vector2UnitY * -1000.0f;
        // Static circle
        world.entities.push_back({});
        entity = &world.entities.back();
        entity->position = { 425.0f, 400.0f };
        entity->gravity_scale = 1.0f;
        entity->collider_type = COLLIDER_TYPE_CIRCLE;
        entity->collider.circle.radius = 20.0f;
        entity->inv_mass = 1.0f / 1.0f;
        entity->net_force += test_force;

        world.entities.push_back({});
        entity = &world.entities.back();
        entity->position = { 425.0f, 400.0f };
        entity->gravity_scale = 1.0f;
        entity->collider_type = COLLIDER_TYPE_CIRCLE;
        entity->collider.circle.radius = 20.0f;
        entity->inv_mass = 1.0f / 100.0f;
        entity->net_force += test_force;
        // Static half-space
        world.entities.push_back({});
        entity = &world.entities.back();
        entity->position = { 400.0f, 600.0f };
        entity->gravity_scale = 0.0f;
        entity->collider_type = COLLIDER_TYPE_HALF_SPACE;
        //entity->collider.half_space.normal = Vector2UnitY * -1.0f;
        entity->collider.half_space.normal = Vector2Rotate(Vector2UnitX, -45.0f * DEG2RAD);
        entity->inv_mass = 0.0f;

        // Static circle
        world.entities.push_back({});
        entity = &world.entities.back();
        entity->position = { 400.0f, 500.0f };
        entity->gravity_scale = 0.0f;
        entity->collider_type = COLLIDER_TYPE_CIRCLE;
        entity->collider.circle.radius = 20.0f;
        entity->inv_mass = 0.0f;

   //     world.entities.push_back({});
   //     entity = &world.entities.back();
   //     entity->position = { 400.0f, 500.0f };
   //     entity->gravity_scale = 0.0f;
   //     entity->collider_type = COLLIDER_TYPE_CIRCLE;
   //     entity->collider.circle.radius = 20.0f;
   //     entity->inv_mass = 0.0f;

   //     // Static half-space
   //     world.entities.push_back({});
   //     entity = &world.entities.back();
   //     entity->position = { 400.0f, 600.0f };
   //     entity->gravity_scale = 0.0f;
   //     entity->collider_type = COLLIDER_TYPE_HALF_SPACE;
   ///*     entity->collider.half_space.normal = Vector2UnitY * -1.0f;*/
   //      entity->collider.half_space.normal = Vector2Rotate(Vector2UnitX, -45.0f * DEG2RAD);
   //      entity->inv_mass = 0.0f;
    }
  
    // Ensure all half-space's have infinite mass (good habit to validate your entities after creation but before physics-loop)
    for (const PhysicsBody& e : world.entities)
    {
        if (e.collider_type == COLLIDER_TYPE_HALF_SPACE)
        {
            assert(IsMassInfinite(e));
        }
    }

    // Delta-time is 0 on the frame 1, which will zero any forces applied before frame 1 (fix by skipping frame 1)
    bool is_first_frame = true;

    InitWindow(800, 800, "Physics-1");
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        if (!is_first_frame)
            Update(world);
        else
            is_first_frame = false;
      /*  float dt = GetFrameTime();*/
       

        // Motion loop
        //for (size_t i = 0; i < world.entities.size(); i++)
        //{
        //    PhysicsBody& e = world.entities[i];
        //    Vector2 acc = world.gravity * e.gravity_scale;

        //    e.velocity += acc * dt;             // v = a * t
        //    e.position += e.velocity * dt;      // p = v * t

        //    // Reset collision render status
        //    e.collision = false;
        //}

        //// Collision loop (Test every object against all other objects)
        //for (size_t i = 0; i < world.entities.size(); i++)
        //{
        //    for (size_t j = i + 1; j < world.entities.size(); j++)
        //    {
        //        PhysicsBody& a = world.entities[i];
        //        PhysicsBody& b = world.entities[j];
        //        assert(a.collider_type != COLLIDER_TYPE_INVALID && b.collider_type != COLLIDER_TYPE_INVALID);
        //        bool collision = false;
        //        Vector2 mtv = Vector2Zeros;

        //        if (a.collider_type == COLLIDER_TYPE_CIRCLE &&
        //            b.collider_type == COLLIDER_TYPE_CIRCLE)
        //        {
        //            collision = CircleCircle(
        //                a.position, a.collider.circle.radius,
        //                b.position, b.collider.circle.radius, &mtv
        //            );
        //        }
        //        else if (
        //            a.collider_type == COLLIDER_TYPE_CIRCLE &&
        //            b.collider_type == COLLIDER_TYPE_HALF_SPACE)
        //        {
        //            collision = CircleHalfSpace(
        //                a.position, a.collider.circle.radius,
        //                b.position, b.collider.half_space.normal, &mtv
        //            );
        //        }
        //        else if (
        //            a.collider_type == COLLIDER_TYPE_HALF_SPACE &&
        //            b.collider_type == COLLIDER_TYPE_CIRCLE)
        //        {
        //            collision = CircleHalfSpace(
        //                b.position, b.collider.circle.radius,
        //                a.position, a.collider.half_space.normal, &mtv
        //            );
        //        }

        //        a.collision |= collision;
        //        b.collision |= collision;
        //        // In the future, we will give our objects mass to determine whether they're immovable (in the case of half-spaces)
        //        // For now, we'll hard-code planes such that the MTV can't move them, only circles!
        //        if (a.collider_type == COLLIDER_TYPE_CIRCLE || b.collider_type == COLLIDER_TYPE_CIRCLE)
        //            a.position += mtv;
        //    }
        //}

        BeginDrawing();
        ClearBackground(WHITE);
        Draw(world);
        //for (const PhysicsBody& e : world.entities)
        //{
        //    Color color = e.collision ? RED : GREEN;
        //    if (e.collider_type == COLLIDER_TYPE_CIRCLE)
        //    {
        //        DrawCircleV(e.position, e.collider.circle.radius, color);
        //    }
        //    else if (e.collider_type == COLLIDER_TYPE_HALF_SPACE)
        //    {
        //        // "Flip" the normal to determine the direction of the half-space
        //        Vector2 direction = { -e.collider.half_space.normal.y, e.collider.half_space.normal.x };
        //        Vector2 p0 = e.position + direction * 1000.0f;
        //        Vector2 p1 = e.position - direction * 1000.0f;
        //        DrawLineEx(p0, p1, 5.0f, color);
        //        DrawLineEx(e.position, e.position + e.collider.half_space.normal * 50.0f, 5.0f, GOLD);
        //    }
        //}

        // Hard-coded visuals for LE4:
       
        //Remove hard coded test to free us of our entity order constraint.
       /* DrawProjCircleHalfSpace(world.entities[2].position, world.entities[2].collider.circle.radius, 
            world.entities[1].position, world.entities[1].collider.half_space.normal);*/

       

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
void DrawProjCircleHalfSpace(Vector2 pos_circle, float rad, Vector2 pos_half_space, Vector2 normal)
{
    Vector2 to_circle = pos_circle - pos_half_space;
    float proj = Vector2DotProduct(to_circle, normal);
    DrawLineEx(pos_half_space, pos_half_space + to_circle, 5.0f, MAGENTA);
    DrawCircleV(pos_half_space + normal * proj, rad, PINK);
    DrawLineEx(pos_half_space, pos_half_space + normal * proj, 4.0f, PINK);
}
// Motion loop
void UpdateMotion(PhysicsWorld& world)
{
    float dt = GetFrameTime();

    for (size_t i = 0; i < world.entities.size(); i++)
    {
        PhysicsBody& e = world.entities[i];

        // F = ma
        // a = F / m        --> possible divide-by-zero error
        // a = F * (1 / m)  --> prevent by multiplying by inverse-mass
        Vector2 acc = e.net_force * e.inv_mass;

        // Acceleration due to gravity is always 9.81 (independent of mass)
        acc += world.gravity * e.gravity_scale;

        e.velocity += acc * dt;             // v = a * t
        e.position += e.velocity * dt;      // p = v * t

        // Reset net force and collision render status
        e.net_force = Vector2Zeros;
        e.collision = false;
    }
}
// Collision loop (Test every object against all other objects)
std::vector<HitPair> DetectCollisions(PhysicsWorld& world)
{
    std::vector<HitPair> collisions;

    for (size_t i = 0; i < world.entities.size(); i++)
    {
        for (size_t j = i + 1; j < world.entities.size(); j++)
        {
            PhysicsBody& a = world.entities[i];
            PhysicsBody& b = world.entities[j];
            assert(a.collider_type != COLLIDER_TYPE_INVALID && b.collider_type != COLLIDER_TYPE_INVALID);
            bool collision = false;
            Vector2 mtv = Vector2Zeros;

            if (a.collider_type == COLLIDER_TYPE_CIRCLE &&
                b.collider_type == COLLIDER_TYPE_CIRCLE)
            {
                collision = CircleCircle(
                    a.position, a.collider.circle.radius,
                    b.position, b.collider.circle.radius,
                    &mtv);
            }
            else if (
                a.collider_type == COLLIDER_TYPE_CIRCLE &&
                b.collider_type == COLLIDER_TYPE_HALF_SPACE)
            {
                collision = CircleHalfSpace(
                    a.position, a.collider.circle.radius,
                    b.position, b.collider.half_space.normal,
                    &mtv);
            }
            else if (
                a.collider_type == COLLIDER_TYPE_HALF_SPACE &&
                b.collider_type == COLLIDER_TYPE_CIRCLE)
            {
                collision = CircleHalfSpace(
                    b.position, b.collider.circle.radius,
                    a.position, a.collider.half_space.normal,
                    &mtv);
            }

            a.collision |= collision;
            b.collision |= collision;

            if (collision)
            {
                HitPair collision;
                collision.a = &a;
                collision.b = &b;
                collision.mtv = mtv;
                collisions.push_back(collision);
            }
        }
    }

    return collisions;
}
 
//void ResolveCollisions(PhysicsWorld& world, std::vector<HitPair> collisions)
void ValidateResolutionVectors(std::vector<HitPair>& collisions)
{
  /*  for (HitPair collision : collisions)*/
    for (HitPair& collision : collisions)
    {
        PhysicsBody* a = collision.a;
        PhysicsBody* b = collision.b;
       /* Vector2 mtv = collision.mtv;*/
        Vector2& mtv = collision.mtv;

        // Ensure at least one entity can move (otherwise we shouldn't be resolving collision)
      /*  assert(!(IsMassInfinite(*a) && IsMassInfinite(*b)));*/
        assert(!(IsMassInfinite(*a) && IsMassInfinite(*b)));

        // Ensure entity A is *ALWAYS* dynamic, and entity B is either static or dynamic
        if (IsMassInfinite(*a))
        {
            // Swap A and B if A is static
            PhysicsBody* temp = b;
            b = a;
            a = temp;
        }

        // Ensure that mtv points FROM B TO A
        Vector2 direction_BA = Vector2Normalize(a->position - b->position);
        float dot = Vector2DotProduct(direction_BA, collision.mtv);
        if (dot < 0.0f)
        {
            mtv *= -1.0f;
        }

        /*if (IsMassInfinite(*b))
        {
            a->position += mtv;
        }
        else
        {
            a->position += mtv * 0.5f;
            b->position -= mtv * 0.5f;
        }*/
    }
}
void ResolveCollisions(std::vector<HitPair> collisions)
{
    for (HitPair collision : collisions)
    {
        PhysicsBody& a = *collision.a;
        PhysicsBody& b = *collision.b;
        Vector2 mtv = collision.mtv;

        if (IsMassInfinite(b))
        {
            a.position += mtv;
        }
        else
        {
            a.position += mtv * 0.5f;
            b.position -= mtv * 0.5f;
        }
    }
}
void Update(PhysicsWorld& world)
{
  /*  UpdateMotion(world);
    std::vector<HitPair> collisions = DetectCollisions(world);
    ResolveCollisions(world, collisions);*/
    UpdateMotion(world);
    std::vector<HitPair> collisions = DetectCollisions(world);
    ValidateResolutionVectors(collisions);
    ResolveCollisions(collisions);
}

void Draw(const PhysicsWorld& world)
{
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

    // Removed hard-coded test to free us of our entity-order constraint!
    //DrawProjCircleHalfSpace(
    //    world.entities[2].position, world.entities[2].collider.circle.radius,
    //    world.entities[1].position, world.entities[1].collider.half_space.normal);
}