#include "physics/body_2d.h"


void Body2D::init(const CreateInfo&)
{
    data.body_id = Physics2D::create_body(this);
}

void Body2D::deinit()
{
    Physics2D::destroy_body(data.body_id);
}

void Body2D::enter()
{
    Physics2D::body_set_residence_mask(data.body_id, data.residence_mask);
    Physics2D::body_set_collision_mask(data.body_id, data.collision_mask);
    Physics2D::body_set_velocity(data.body_id, data.velocity);
    Physics2D::body_set_friction(data.body_id, data.mass);
    Physics2D::body_set_mass(data.body_id, data.friction);
}

void Body2D::exit()
{
}


void Body2D::set_type(Body2D::BodyType new_type)
{
    if (data.type != new_type)
    {
        data.type = new_type;
        switch (new_type)
        {
        case BodyType::STATIC:
            set_mass(0);
            break;
        case BodyType::DYNAMIC:
        case BodyType::KINEMATIC:
            set_mass(data.mass >= 0 ? data.mass : 1);
            set_velocity(Vector2(1, 1));
            break;
        default:
            DebugAssert(false, "invalid body type");
        }

        if (data.body_id != Physics2D::BodyID::InvalidID)
        {
            Physics2D::body_set_type(data.body_id, Physics2D::BodyType(new_type));
        }
    }
}

void Body2D::shape_as_box(const Vector2& size)
{
    if (data.body_id != Physics2D::BodyID::InvalidID)
    {
        Physics2D::body_shape_as_box(data.body_id, size);
    }
}

void Body2D::set_velocity(const Vector2& new_velocity)
{
    data.velocity = new_velocity;
    if (data.body_id != Physics2D::BodyID::InvalidID)
    {
        Physics2D::body_set_velocity(data.body_id, new_velocity);
    }
}

void Body2D::set_mass(f32 new_mass)
{
    data.mass = new_mass;
    if (data.body_id != Physics2D::BodyID::InvalidID)
    {
        Physics2D::body_set_mass(data.body_id, new_mass);
    }
}

void Body2D::set_friction(f32 new_friction)
{
    data.friction = new_friction;
    if (data.body_id != Physics2D::BodyID::InvalidID)
    {
        Physics2D::body_set_friction(data.body_id, new_friction);
    }
}

void Body2D::apply_force(const Vector2& point, const Vector2& force) const
{
    if (data.body_id != Physics2D::BodyID::InvalidID)
    {
        Physics2D::body_apply_force(data.body_id, point, force);
    }
}

void Body2D::apply_impulse(const Vector2& point, const Vector2& force) const
{
    if (data.body_id != Physics2D::BodyID::InvalidID)
    {
        Physics2D::body_apply_impulse(data.body_id, point, force);
    }
}

void Body2D::set_fixed_rotation(bool enable) const
{
    if (data.body_id != Physics2D::BodyID::InvalidID)
    {
        Physics2D::body_set_fixed_rotation(data.body_id, enable);
    }
}

void Body2D::set_residence_mask(CollisionMask mask)
{
    data.residence_mask = mask;
    if (data.body_id != Physics2D::BodyID::InvalidID)
    {
        Physics2D::body_set_residence_mask(data.body_id, mask);
    }
}

void Body2D::set_collision_mask(CollisionMask mask)
{
    data.collision_mask = mask;
    if(data.body_id != Physics2D::BodyID::InvalidID)
    {
        Physics2D::body_set_collision_mask(data.body_id, mask);
    }
}
