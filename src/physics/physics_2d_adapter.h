#pragma once
#include "physics/physics_2d.h"


namespace InternalPhysics2D
{

struct Adapter
{
    virtual void initialize(Mem::Allocator* allocator) = 0;
    virtual void shutdown() = 0;

    virtual void step(f32 dt) = 0;

    virtual Physics2D::BodyID body_create(Opaque* user_data) = 0;
    virtual void body_destroy(Physics2D::BodyID body) = 0;

    // Body
    virtual void body_set_shape(Physics2D::BodyID body, const Shape2D& new_shape) = 0;
    virtual Shape2D body_get_shape(Physics2D::BodyID body) = 0;

    virtual void body_set_user_data(Physics2D::BodyID body, Opaque* new_user_data) = 0;
    virtual Opaque* body_get_user_data(Physics2D::BodyID body) = 0;
    virtual void body_set_transform(Physics2D::BodyID body, const Transform2D& new_transform) = 0;
    virtual Transform2D body_get_transform(Physics2D::BodyID body) = 0;
    virtual void body_set_type(Physics2D::BodyID body, Physics2D::BodyType new_type) = 0;
    virtual void body_set_velocity(Physics2D::BodyID body, const Vector2& new_velocity) = 0;
    virtual Vector2 body_get_velocity(Physics2D::BodyID body) = 0;
    virtual void body_set_angular_velocity(Physics2D::BodyID body, f32 angular_velocity) = 0;
    virtual f32 body_get_angular_velocity(Physics2D::BodyID body) = 0;
    virtual void body_set_mass(Physics2D::BodyID body, f32 new_mass) = 0;
    virtual f32 body_get_mass(Physics2D::BodyID body) = 0;
    virtual void body_set_friction(Physics2D::BodyID body, f32 new_friction) = 0;
    virtual f32 body_get_friction(Physics2D::BodyID body) = 0;
    virtual void body_set_air_friction(Physics2D::BodyID body, f32 new_air_friction) = 0;
    virtual f32 body_get_air_friction(Physics2D::BodyID body) = 0;
    virtual void body_set_restitution(Physics2D::BodyID body, f32 new_restitution) = 0;
    virtual f32 body_get_restitution(Physics2D::BodyID body) = 0;
    virtual void body_apply_force(Physics2D::BodyID body, const Vector2& force, const Vector2& point) = 0;
    virtual void body_apply_impulse(Physics2D::BodyID body, const Vector2& impulse, const Vector2& point) = 0;
    virtual void body_set_fixed_rotation(Physics2D::BodyID body, bool enable) = 0;
    virtual bool body_is_on_floor(Physics2D::BodyID body) = 0;
    virtual bool body_is_on_ceil(Physics2D::BodyID body) = 0;

    virtual void body_set_residence_mask(Physics2D::BodyID body_id, Physics2D::CollisionMask mask) = 0;
    virtual Physics2D::CollisionMask body_get_residence_mask(Physics2D::BodyID body_id) = 0;
    virtual void body_set_collision_mask(Physics2D::BodyID body_id, Physics2D::CollisionMask mask) = 0;
    virtual Physics2D::CollisionMask body_get_collision_mask(Physics2D::BodyID body_id) = 0;
    virtual void body_set_on_collide(Physics2D::BodyID body_id, Physics2D::EventOnCollide on_collide) = 0;

    // Area
    virtual Physics2D::AreaID area_create(Opaque* user_data) = 0;
    virtual void area_destroy(Physics2D::AreaID area) = 0;
    
    virtual void area_set_shape(Physics2D::AreaID area, const Shape2D& new_shape) = 0;
    virtual Shape2D area_get_shape(Physics2D::AreaID area) = 0;

    virtual void area_set_user_data(Physics2D::AreaID area, Opaque* new_user_data) = 0;
    virtual Opaque* area_get_user_data(Physics2D::AreaID area) = 0;
    virtual void area_set_transform(Physics2D::AreaID area, const Transform2D& new_transform) = 0;
    virtual Transform2D area_get_transform(Physics2D::AreaID area) = 0;
    virtual void area_set_residence_mask(Physics2D::AreaID area, Physics2D::CollisionMask mask) = 0;
    virtual Physics2D::CollisionMask area_get_residence_mask(Physics2D::AreaID area) = 0;

    virtual void area_set_on_body_enter(Physics2D::AreaID area, Physics2D::EventOnBodyEnter on_body_enter) = 0;
    virtual void area_set_on_body_exit(Physics2D::AreaID area, Physics2D::EventOnBodyExit on_body_exit) = 0;

    // Internal
    virtual void property_change(StringView property_name, PropertyValue new_value) = 0;
};

}
