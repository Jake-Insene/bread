#pragma once
#include "collections/array.h"

#include "physics/p2d/p2d_shape.h"
#include "physics/p2d/p2d_types.h"


struct [[nodiscard]] P2DBody
{
    struct InternalData
    {
        Vector2 velocity;
        Vector2 force_accumulator;
        f32 torque_accumulator;
        f32 angular_velocity;
        
        f32 inertia;
        f32 inv_inertia;
        f32 mass;
        f32 inv_mass;
        f32 friction;
        f32 air_friction;
        f32 restitution;

        P2DShape shape_transformed;
        Transform2D transform;
    } data;

    P2DShape shape;

    Opaque* user_data;
    Physics2D::BodyID self;
    Physics2D::BodyType type;
    Physics2D::CollisionMask residence_mask;
    Physics2D::CollisionMask collision_mask;

    Physics2D::EventOnCollide on_collide;

    bool fixed_rotation;
    bool is_on_floor;
    bool is_on_ceil;
    bool moved;
    
    Array<PhysicsTileCoord> tiles_on;

    void init(const mem::Allocator allocator, Physics2D::BodyID id, Opaque* ud);
    void destroy();

    void apply_force(const Vector2& force, const Vector2& point);

    void add_force(const Vector2& force);
    Vector2 get_force() const;

    void set_velocity(const Vector2& new_velocity);
    Vector2 get_velocity() const;
    void add_velocity(const Vector2& vel);

    void set_angular_velocity(f32 new_angular_velocity);
    f32 get_angular_velocity() const;
    void add_angular_velocity(f32 ang_vel);

    [[nodiscard]] f32 get_inv_inertia() const {return data.inv_inertia; }

    void set_mass(f32 new_mass);
    [[nodiscard]] f32 get_mass() const { return data.mass; }
    [[nodiscard]] f32 get_inv_mass() const
    {
        if (type == Physics2D::STATIC || type == Physics2D::KINEMATIC) return 0.0f;
        return data.inv_mass;
    }

    void set_friction(f32 new_friction);
    [[nodiscard]] f32 get_friction() const { return data.friction; }

    void set_air_friction(f32 new_air_friction);
    [[nodiscard]] f32 get_air_friction() const { return data.air_friction; }

    void set_restitution(f32 new_restitution);
    [[nodiscard]] f32 get_restitution() const { return data.restitution; }

    void set_shape_from_2d(const Shape2D& new_shape);

    const P2DShape& get_shape_transformed() const { return data.shape_transformed; }
    void set_transform(const Transform2D& new_transform);
    const Transform2D& get_transform() const { return data.transform; }

    void step(f32 dt);
    void integrate(f32 dt);
    
    void _compute_inertia();
    void _semi_implicit_euler(f32 dt);

};