#pragma once
#include "physics/physics_2d.h"


namespace InternalPhysics2D
{

struct Adapter
{
    VTFunc(void, initialize, const mem::Allocator&);
    VTFunc(void, shutdown);

    VTFunc(void, step, f32);

    VTFunc(Physics2D::BodyID, body_create, Opaque*);
    VTFunc(void, body_destroy, Physics2D::BodyID);
    VTFunc(Physics2D::AreaID, area_create, Opaque*);
    VTFunc(void, area_destroy, Physics2D::AreaID);

    VTFunc(void, body_set_shape, Physics2D::BodyID, const Shape2D&);
    VTFunc(Shape2D, body_get_shape, Physics2D::BodyID);

    VTFunc(void, body_set_user_data, Physics2D::BodyID, Opaque*);
    VTFunc(Opaque*, body_get_user_data, Physics2D::BodyID);
    VTFunc(void, body_set_transform, Physics2D::BodyID, const Transform2D&);
    VTFunc(Transform2D, body_get_transform, Physics2D::BodyID);
    VTFunc(void, body_set_type, Physics2D::BodyID, Physics2D::BodyType);
    VTFunc(void, body_set_velocity, Physics2D::BodyID, const Vector2&);
    VTFunc(Vector2, body_get_velocity, Physics2D::BodyID);
    VTFunc(void, body_set_angular_velocity, Physics2D::BodyID, f32);
    VTFunc(f32, body_get_angular_velocity, Physics2D::BodyID);
    VTFunc(void, body_set_mass, Physics2D::BodyID, f32);
    VTFunc(f32, body_get_mass, Physics2D::BodyID);
    VTFunc(void, body_set_friction, Physics2D::BodyID, f32);
    VTFunc(f32, body_get_friction, Physics2D::BodyID);
    VTFunc(void, body_set_air_friction, Physics2D::BodyID, f32);
    VTFunc(f32, body_get_air_friction, Physics2D::BodyID);
    VTFunc(void, body_set_restitution, Physics2D::BodyID, f32);
    VTFunc(f32, body_get_restitution, Physics2D::BodyID);
    VTFunc(void, body_apply_force, Physics2D::BodyID, const Vector2&, const Vector2&);
    VTFunc(void, body_apply_impulse, Physics2D::BodyID, const Vector2&, const Vector2&);
    VTFunc(void, body_set_fixed_rotation, Physics2D::BodyID, bool);
    VTFunc(bool, body_is_on_floor, Physics2D::BodyID);
    VTFunc(bool, body_is_on_ceil, Physics2D::BodyID);
    VTFunc(void, body_set_residence_mask, Physics2D::BodyID, Physics2D::CollisionMask);
    VTFunc(Physics2D::CollisionMask, body_get_residence_mask, Physics2D::BodyID);
    VTFunc(void, body_set_collision_mask, Physics2D::BodyID, Physics2D::CollisionMask);
    VTFunc(Physics2D::CollisionMask, body_get_collision_mask, Physics2D::BodyID);
    VTFunc(void, body_set_on_collide, Physics2D::BodyID, Physics2D::EventOnCollide);

    VTFunc(void, area_set_shape, Physics2D::AreaID, const Shape2D&);
    VTFunc(Shape2D, area_get_shape, Physics2D::AreaID);

    VTFunc(void, area_set_user_data, Physics2D::AreaID, Opaque*);
    VTFunc(Opaque*, area_get_user_data, Physics2D::AreaID);
    VTFunc(void, area_set_transform, Physics2D::AreaID, const Transform2D&);
    VTFunc(Transform2D, area_get_transform, Physics2D::AreaID);
    VTFunc(void, area_set_residence_mask, Physics2D::AreaID, Physics2D::CollisionMask);
    VTFunc(Physics2D::CollisionMask, area_get_residence_mask, Physics2D::AreaID);

    VTFunc(void, area_set_on_body_enter, Physics2D::AreaID, Physics2D::EventOnBodyEnter);
    VTFunc(void, area_set_on_body_exit, Physics2D::AreaID, Physics2D::EventOnBodyExit);

    // Internal
    VTFunc(void, property_change, StringView, PropertyValue);
};

}
