#pragma once
#include "collections/array.h"
#include "physics/physics_2d.h"

#include "physics/p2d/p2d_shape.h"
#include "physics/p2d/p2d_types.h"


struct [[nodiscard]] P2DBody
{
    struct InternalData
    {
        Vector2 velocity;
        Vector2 force_accumulator;
        f32 angular_velocity;
        
        f32 inertia;
        f32 inv_inertia;
        f32 mass;
        f32 inv_mass;
        f32 friction;
        f32 air_friction;
        f32 bounce;

        P2DShape shape;
    };

    Object2D* target;
    Physics2D::BodyID self;
    Physics2D::BodyType type;
    Physics2D::CollisionMask residence_mask;
    Physics2D::CollisionMask collision_mask;

    void* _this;
    Physics2D::EventOnCollide on_collide;

    InternalData data;

    bool fixed_rotation;
    bool is_on_floor;
    bool is_on_ceil;
    bool moved;

    Array<PhysicsTileCoord> tiles_on;

    void add_force(const Vector2& force);
    Vector2 get_force() const;

    void set_velocity(const Vector2& new_velocity);
    Vector2 get_velocity() const;

    void set_angular_velocity(f32 new_angular_velocity);
    f32 get_angular_velocity() const;

    [[nodiscard]] f32 get_inv_inertia() const {return data.inv_inertia; }

    void set_mass(f32 new_mass);
    [[nodiscard]] f32 get_mass() const { return data.mass; }
    [[nodiscard]] f32 get_inv_mass() const { return data.inv_mass; }

    void set_friction(f32 new_friction);
    [[nodiscard]] f32 get_friction() const { return data.friction; }

    void set_air_friction(f32 new_air_friction);
    [[nodiscard]] f32 get_air_friction() const { return data.air_friction; }

    void set_bounce(f32 new_bounce);
    [[nodiscard]] f32 get_bounce() const { return data.bounce; }

    void set_shape(const P2DShape& new_shape);
    const P2DShape& get_shape() const;

    void step(f32 dt);
    void integrate(f32 dt);
    void compute_inertia();

    void _semi_implicit_euler(f32 dt);

};