#pragma once
#include "2d/object_2d.h"

#include "debug/debug.h"
#include "physics/physics_2d.h"


struct Body2D : Object2D
{
    OBJECT(Body2D, Object2D);

    static void _bind_vtable(Object2D::VTable& vtable);

    using BodyType = Physics2D::BodyType;
    using CollisionMask = Physics2D::CollisionMask;
    
    struct InternalData
    {
        Physics2D::BodyID body_id = Physics2D::BodyID::InvalidID;
        BodyType type = BodyType::UNKNOWN;
        
        f32 mass = 1.f;
        f32 friction = 1.f;
        f32 air_friction = 1.f;
        f32 bounce = 1.f;

        bool grounded = false;

        CollisionMask residence_mask = Physics2D::DEFAULT_COLLISION_MASK;
        CollisionMask collision_mask = Physics2D::DEFAULT_COLLISION_MASK;
    } data;

    Event<void(Object::*)(Object2D*)> on_collide;
    
    void init(const CreateInfo&);
    void deinit();
    
    void enter();
    void exit();

    void transform_changed();
    
    void set_type(Body2D::BodyType new_type);
    Body2D::BodyType get_type() const { return data.type;}
    
    void set_shape(const Shape2D& shape);
    Shape2D get_shape();

    void set_velocity(const Vector2& new_velocity);
    Vector2 get_velocity() const;

    void set_angular_velocity(f32 angular_velocity) const;
    [[nodiscard]] f32 get_angular_velocity() const;

    void set_mass(f32 new_mass);
    [[nodiscard]] f32 get_mass() const { return data.mass; }

    void set_friction(f32 new_friction);
    [[nodiscard]] f32 get_friction() const { return data.friction; }

    void set_air_friction(f32 new_air_friction);
    [[nodiscard]] f32 get_air_friction() const { return data.air_friction; }

    void set_bounce(f32 new_bounce);
    [[nodiscard]] f32 get_bounce() const;

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
