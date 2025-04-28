#pragma once
#include "physics/physics_2d.h"
#include "physics/p2d/shape_2d.h"


struct P2DDriver
{
    struct BodyFlags
    {
        u32 fixed_rotation : 1;
    };

    struct [[nodiscard]] Body
    {
        Body2D* target;
        Physics2D::BodyID self;
        Shape2D shape;

        Vector2 last_updated_pos;

        Physics2D::BodyType type;
        Vector2 velocity;
        f32 mass;
        f32 friction;

        BodyFlags flags;
    };

    struct InternalData
    {
        mem::Allocator allocator;

        Vector2 gravity;

        QueueArray<Body, Physics2D::BodyID> current_bodies;
        Array<Physics2D::BodyID> process_bodies;
    };

    static inline InternalData data;

    static Physics2D::VTable get_vtable();

    [[nodiscard]] static mem::Allocator& get_allocator()
    {
        return data.allocator;
    }
    [[nodiscard]] static Body& get_body(Physics2D::BodyID bodyid)
    {
        return data.current_bodies.get(bodyid);
    }

    static void initialize(mem::Allocator& allocator);
    static void shutdown();

    static void step(f32 dt);

    static Physics2D::BodyID create_body(Body2D* object_body);
    static void destroy_body(Physics2D::BodyID bodyid);

    static void body_as_box(Physics2D::BodyID bodyid, const Vector2& new_size);
    static void body_set_type(Physics2D::BodyID bodyid, Physics2D::BodyType new_type);
    static void body_set_velocity(Physics2D::BodyID bodyid, const Vector2& new_velocity);
    static Vector2 body_get_velocity(Physics2D::BodyID bodyid);
    static void body_set_mass(Physics2D::BodyID bodyid, f32 new_mass);
    static f32 body_get_mass(Physics2D::BodyID bodyid);
    static void body_set_friction(Physics2D::BodyID bodyid, f32 new_friction);
    static f32 body_get_friction(Physics2D::BodyID bodyid);
    static void body_apply_force(Physics2D::BodyID bodyid, const Vector2& point, const Vector2& force);
    static void body_apply_impulse(Physics2D::BodyID bodyid, const Vector2& point, const Vector2& force);
    static void body_set_fixed_rotation(Physics2D::BodyID bodyid, bool enable);
};
