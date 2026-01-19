#include "physics/physics_2d.h"

#include "physics/p2d/p2d_driver.h"


void Physics2D::initialize(const mem::Allocator& allocator, Physics2D::DriverType)
{
    data.allocator = allocator;

    // The implementation could need this in initialization
    data.properties = StringMap<PropertyValue>::with_size(data.allocator, 4);
    data.properties.insert("/gravity", Property::Vector2(0, -98));
    data.properties.insert("/debug_draw", Property::Bool(false));
    data.properties.insert("/tile_size", Property::Integer(64));
    data.properties.insert("/fixed_step", Property::Float(1.0f / 60.0f));

    P2DDriver::initialize(allocator);
}

void Physics2D::shutdown()
{
    P2DDriver::shutdown();

    data.properties.destroy();
}

void Physics2D::step(f32 delta_time)
{
    P2DDriver::step(delta_time);
}

Physics2D::BodyID Physics2D::create_body(Opaque* user_data)
{
    return P2DDriver::create_body(user_data);
}

void Physics2D::destroy_body(BodyID body_id)
{
    P2DDriver::destroy_body(body_id);
}

Physics2D::AreaID Physics2D::create_area(Opaque* user_data)
{
    return P2DDriver::create_area(user_data);
}

void Physics2D::destroy_area(AreaID area_id)
{
    P2DDriver::destroy_area(area_id);
}

void Physics2D::body_set_shape(BodyID body_id, const Shape2D& new_shape)
{
    P2DDriver::body_set_shape(body_id, new_shape);
}

Shape2D Physics2D::body_get_shape(BodyID body_id)
{
    return P2DDriver::body_get_shape(body_id);
}

void Physics2D::body_set_user_data(BodyID body_id, Opaque* user_data)
{
    P2DDriver::body_set_user_data(body_id, user_data);
}

Opaque* Physics2D::body_get_user_data(BodyID body_id)
{
    return P2DDriver::body_get_user_data(body_id);
}

void Physics2D::body_set_transform(BodyID body_id, const Transform2D& new_transform)
{
    P2DDriver::body_set_transform(body_id, new_transform);
}

Transform2D Physics2D::body_get_transform(BodyID body_id)
{
    return P2DDriver::body_get_transform(body_id);
}

void Physics2D::body_set_type(BodyID body_id, BodyType new_type)
{
    P2DDriver::body_set_type(body_id, new_type);
}

void Physics2D::body_set_velocity(BodyID body_id, const Vector2& new_velocity)
{
    P2DDriver::body_set_velocity(body_id, new_velocity);
}

Vector2 Physics2D::body_get_velocity(BodyID body_id)
{
    return P2DDriver::body_get_velocity(body_id);
}

void Physics2D::body_set_angular_velocity(BodyID body_id, f32 angular_velocity)
{
    P2DDriver::body_set_angular_velocity(body_id, angular_velocity);
}

f32 Physics2D::body_get_angular_velocity(BodyID body_id)
{
    return P2DDriver::body_get_angular_velocity(body_id);
}

void Physics2D::body_set_mass(BodyID body_id, f32 mass)
{
    P2DDriver::body_set_mass(body_id, mass);
}

f32 Physics2D::body_get_mass(BodyID body_id)
{
    return P2DDriver::body_get_mass(body_id);
}

void Physics2D::body_set_friction(BodyID body_id, f32 friction)
{
    P2DDriver::body_set_friction(body_id, friction);
}

f32 Physics2D::body_get_friction(BodyID body_id)
{
    return P2DDriver::body_get_friction(body_id);
}

void Physics2D::body_set_air_friction(BodyID body_id, f32 air_friction)
{
    P2DDriver::body_set_air_friction(body_id, air_friction);
}

f32 Physics2D::body_get_air_friction(BodyID body_id)
{
    return P2DDriver::body_get_air_friction(body_id);
}

void Physics2D::body_set_restitution(BodyID body_id, f32 new_restitution)
{
    P2DDriver::body_set_restitution(body_id, new_restitution);
}

f32 Physics2D::body_get_restitution(BodyID body_id)
{
    return P2DDriver::body_get_restitution(body_id);
}

void Physics2D::body_apply_force(BodyID body_id, const Vector2& force, const Vector2& point)
{
    P2DDriver::body_apply_force(body_id, force, point);
}

void Physics2D::body_apply_impulse(BodyID body_id, const Vector2& impulse, const Vector2& point)
{
    P2DDriver::body_apply_impulse(body_id, impulse, point);
}

void Physics2D::body_set_fixed_rotation(BodyID body_id, bool fixed_rotation)
{
    P2DDriver::body_set_fixed_rotation(body_id, fixed_rotation);
}

bool Physics2D::body_is_on_floor(BodyID body_id)
{
    return P2DDriver::body_is_on_floor(body_id);
}

bool Physics2D::body_is_on_ceil(BodyID body_id)
{
    return P2DDriver::body_is_on_ceil(body_id);
}

void Physics2D::body_set_residence_mask(BodyID body_id, CollisionMask mask)
{
    P2DDriver::body_set_residence_mask(body_id, mask);
}

Physics2D::CollisionMask Physics2D::body_get_residence_mask(BodyID body_id)
{
    return P2DDriver::body_get_residence_mask(body_id);
}

void Physics2D::body_set_collision_mask(BodyID body_id, CollisionMask mask)
{
    P2DDriver::body_set_collision_mask(body_id, mask);
}

Physics2D::CollisionMask Physics2D::body_get_collision_mask(BodyID body_id)
{
    return P2DDriver::body_get_collision_mask(body_id);
}

void Physics2D::body_set_on_collide(BodyID body_id, EventOnCollide event)
{
    P2DDriver::body_set_on_collide(body_id, event);
}

void Physics2D::area_set_shape(AreaID area_id, const Shape2D& shape)
{
    P2DDriver::area_set_shape(area_id, shape);
}

Shape2D Physics2D::area_get_shape(AreaID area_id)
{
    return P2DDriver::area_get_shape(area_id);
}

void Physics2D::area_set_user_data(AreaID area_id, Opaque* user_data)
{
    P2DDriver::area_set_user_data(area_id, user_data);
}

Opaque* Physics2D::area_get_user_data(AreaID area_id)
{
    return P2DDriver::area_get_user_data(area_id);
}

void Physics2D::area_set_transform(AreaID area_id, const Transform2D& transform)
{
    P2DDriver::area_set_transform(area_id, transform);
}

Transform2D Physics2D::area_get_transform(AreaID area_id)
{
    return P2DDriver::area_get_transform(area_id);
}

void Physics2D::area_set_residence_mask(AreaID area_id, CollisionMask mask)
{
    P2DDriver::area_set_residence_mask(area_id, mask);
}

Physics2D::CollisionMask Physics2D::area_get_residence_mask(AreaID area_id)
{
    return P2DDriver::area_get_residence_mask(area_id);
}

void Physics2D::area_set_on_body_enter(AreaID area_id, EventOnBodyEnter event)
{
    P2DDriver::area_set_on_body_enter(area_id, event);
}

void Physics2D::area_set_on_body_exit(AreaID area_id, EventOnBodyExit event)
{
    P2DDriver::area_set_on_body_exit(area_id, event);
}

void Physics2D::set_property(StringView property_name, PropertyValue new_value)
{
    data.properties.insert(property_name, new_value);
    P2DDriver::property_change(property_name, new_value);
}

PropertyValue Physics2D::get_property(StringView property_name)
{
    return data.properties.get(property_name);
}
