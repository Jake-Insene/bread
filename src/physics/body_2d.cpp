#include "physics/body_2d.h"


void Body2D::init(const CreateInfo&)
{
    data.body_id = Physics2D::create_body(this);
    Physics2D::EventOnCollide event_on_collide;
    event_on_collide.bind(&_on_body_collide);

    Physics2D::body_set_on_collide(data.body_id, this, event_on_collide);
}

void Body2D::deinit()
{
    Physics2D::destroy_body(data.body_id);
}

void Body2D::enter()
{}

void Body2D::exit()
{
}

void Body2D::set_type(Body2D::BodyType new_type)
{
    if (data.type == new_type)
        return;

    data.type = new_type;
    Physics2D::body_set_type(data.body_id, Physics2D::BodyType(new_type));
    data.mass = Physics2D::body_get_mass(data.body_id);
    data.friction = Physics2D::body_get_friction(data.body_id);
    data.air_friction = Physics2D::body_get_air_friction(data.body_id);
}

void Body2D::set_shape(const Shape2D& shape)
{
    Physics2D::body_set_shape(data.body_id, shape);
}

Shape2D Body2D::get_shape()
{
    return Physics2D::body_get_shape(data.body_id);
}

void Body2D::set_velocity(const Vector2& new_velocity)
{
    Physics2D::body_set_velocity(data.body_id, new_velocity);
}

Vector2 Body2D::get_velocity() const
{
    return Physics2D::body_get_velocity(data.body_id);
}

void Body2D::set_mass(f32 new_mass)
{
    data.mass = new_mass;
    Physics2D::body_set_mass(data.body_id, new_mass);
}

void Body2D::set_friction(f32 new_friction)
{
    data.friction = new_friction;
    Physics2D::body_set_friction(data.body_id, new_friction);
}

void Body2D::set_air_friction(f32 new_air_friction)
{
    data.air_friction = new_air_friction;
    Physics2D::body_set_air_friction(data.body_id, new_air_friction);
}

void Body2D::set_bounce(f32 new_bounce)
{
    data.bounce = new_bounce;
    Physics2D::body_set_bounce(data.body_id, new_bounce);
}

f32 Body2D::get_bounce() const
{
    return Physics2D::body_get_bounce(data.body_id);
}

void Body2D::apply_force(const Vector2& point, const Vector2& force) const
{
    Physics2D::body_apply_force(data.body_id, point, force);
}

void Body2D::apply_impulse(const Vector2& point, const Vector2& force) const
{
    Physics2D::body_apply_impulse(data.body_id, point, force);
}

void Body2D::set_fixed_rotation(bool enable) const
{
    Physics2D::body_set_fixed_rotation(data.body_id, enable);
}

void Body2D::set_residence_mask(CollisionMask mask)
{
    data.residence_mask = mask;
    Physics2D::body_set_residence_mask(data.body_id, mask);
}

void Body2D::set_collision_mask(CollisionMask mask)
{
    data.collision_mask = mask;
    Physics2D::body_set_collision_mask(data.body_id, mask);
}

void Body2D::_on_body_collide(void* _this, Object2D* obj)
{
    Body2D* body = (Body2D*)_this;

    if (body->on_collide.has_func() == false)
        return;

    body->on_collide.call(obj);
}
