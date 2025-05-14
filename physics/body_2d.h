#pragma once
#include "2d/object_2d.h"

#include "debug/debug.h"
#include "physics/physics_2d.h"


struct Body2D : Object2D
{
    OBJECT(Body2D, Object2D);

    using BodyType = Physics2D::BodyType;
    using CollisionMask = Physics2D::CollisionMask;
    
    // As everything in a struct is public we need to hide data
    // that should not be modified/access directly, this also
    // resolve some namespace problems.
    struct InternalData
    {
        Physics2D::BodyID body_id = Physics2D::BodyID::InvalidID;
        BodyType type = BodyType::DYNAMIC;
        
        Vector2 velocity{0, 0};
        f32 mass = 1;
        f32 friction = 1;

        bool grounded = false;

        CollisionMask residence_mask = CollisionMask::COLLISION_MASK_0;
        CollisionMask collision_mask = CollisionMask::COLLISION_MASK_0;
    } data;

    Event<void(Body2D::*)(Body2D*)> colliding_with;
    
    void init(const CreateInfo&);
    void deinit();
    
    void start();
    void exit();
    
    void set_type(Body2D::BodyType new_type);
    Body2D::BodyType get_type() const { return data.type;}
    
    void as_box(const Vector2& size);
    
    void set_velocity(const Vector2& new_velocity);
    Vector2 get_velocity() const { return data.velocity; }

    void set_mass(f32 new_mass);
    [[nodiscard]] f32 get_mass() const { return data.mass; }

    void set_friction(f32 new_friction);
    [[nodiscard]] f32 get_friction() const { return data.friction; }

    void apply_force(const Vector2& point, const Vector2& force) const;
    void apply_impulse(const Vector2& point, const Vector2& force) const;
    void set_fixed_rotation(bool enable) const;

    [[nodiscard]] bool is_on_floor() const { return Physics2D::body_is_on_floor(data.body_id); }

    void set_residence_mask(CollisionMask mask);
    [[nodiscard]] CollisionMask get_residence_mask() const { return data.residence_mask; }

    void set_collision_mask(CollisionMask mask);
    [[nodiscard]] CollisionMask get_collision_mask() const { return data.collision_mask; }
};
