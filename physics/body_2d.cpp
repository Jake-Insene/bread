#include "physics/body_2d.h"


void Body2D::init(const CreateInfo&)
{}

void Body2D::deinit()
{
}

void Body2D::start()
{
    data.body_id = Physics2D::create_body(this);
}

void Body2D::exit()
{
    Physics2D::destroy_body(data.body_id);
}


void Body2D::set_type(Body2D::BodyType new_type)
{
    if(data.type != new_type)
    {
        data.type = new_type;
        switch (new_type)
        {
        case Body2D::STATIC:
            set_mass(0);
            break;
        case Body2D::DYNAMIC:
        case Body2D::KINEMATIC:
            set_mass(data.mass >= 0 ? data.mass : 1);
            set_velocity(Vector2(1, 1));
            break;
        default:
            DebugAssert(false, "Invalid body type");
        }
        Physics2D::body_set_type(data.body_id, Physics2D::BodyType(new_type));
    }
}

void Body2D::as_rect(const Vector2& size)
{
    Physics2D::body_as_box(data.body_id, size);
}

void Body2D::set_velocity(const Vector2& new_velocity)
{
    data.velocity = new_velocity;
    Physics2D::body_set_velocity(data.body_id, new_velocity);
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
