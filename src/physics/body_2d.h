#pragma once
#include "2d/object_2d.h"

#include "debug/debug.h"
#include "physics/physics_2d.h"


struct Body2D : Object2D
{
    OBJECT(Body2D, Object2D);

    using BodyType = Physics2D::BodyType;
    using CollisionMask = Physics2D::CollisionMask;
    
    struct InternalData
    {
        Physics2D::BodyID body_id = Physics2D::BodyID::InvalidID;
        BodyType type = BodyType::UNKNOWN;
        
        Vector2 velocity{0, 0};
        f32 mass = 1;
        f32 friction = 1;
        f32 air_friction = 1;

        bool grounded = false;

        CollisionMask residence_mask = Physics2D::DEFAULT_COLLISION_MASK;
        CollisionMask collision_mask = Physics2D::DEFAULT_COLLISION_MASK;
    } data;

    Event<void(Object::*)(Object2D*)> on_collide;
    
    void init(const CreateInfo&);
    void deinit();
    
    void enter();
    void exit();
    
    void set_type(Body2D::BodyType new_type);
    Body2D::BodyType get_type() const { return data.type;}
    
    void add_shape(const Shape2D& new_shape);
    void remove_shape(usize index);
    usize get_shape_count();
    void set_shape(usize index, const Shape2D& shape);
    Shape2D get_shape(usize index);
    
    void set_velocity(const Vector2& new_velocity);
    Vector2 get_velocity() const { return data.velocity; }

    void set_mass(f32 new_mass);
    [[nodiscard]] f32 get_mass() const { return data.mass; }

    void set_friction(f32 new_friction);
    [[nodiscard]] f32 get_friction() const { return data.friction; }

    void set_air_friction(f32 new_air_friction);
    [[nodiscard]] f32 get_air_friction() const { return data.air_friction; }

    void apply_force(const Vector2& point, const Vector2& force) const;
    void apply_impulse(const Vector2& point, const Vector2& force) const;
    void set_fixed_rotation(bool enable) const;

    [[nodiscard]] bool is_on_floor() const { return Physics2D::body_is_on_floor(data.body_id); }
    [[nodiscard]] bool is_on_ceil() const { return Physics2D::body_is_on_ceil(data.body_id); }

    void set_residence_mask(CollisionMask mask);
    [[nodiscard]] CollisionMask get_residence_mask() const { return data.residence_mask; }

    void set_collision_mask(CollisionMask mask);
    [[nodiscard]] CollisionMask get_collision_mask() const { return data.collision_mask; }

    static void _on_body_collide(void* _this, Object2D* obj);
};
