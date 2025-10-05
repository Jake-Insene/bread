#include "physics/p2d/p2d_body.h"

#include "2d/object_2d.h"



void P2DBody::add_force(const Vector2& force)
{
	data.force_accumulator += force;
}

Vector2 P2DBody::get_force() const
{
	return data.force_accumulator;
}

void P2DBody::set_velocity(const Vector2& new_velocity)
{
	data.velocity = new_velocity;
}

Vector2 P2DBody::get_velocity() const
{
	return data.velocity;
}

void P2DBody::set_angular_velocity(f32 new_angular_velocity)
{
	data.angular_velocity = new_angular_velocity;
}

f32 P2DBody::get_angular_velocity() const
{
	return data.angular_velocity;
}

void P2DBody::set_mass(f32 new_mass)
{
	if (new_mass > 0)
	{
		data.mass = new_mass;
		data.inv_mass = 1.f / new_mass;
	}
	else
	{
		data.mass = 0.f;
		data.inv_mass = 0.f;
	}
}

void P2DBody::set_friction(f32 new_friction)
{
	data.friction = new_friction;
}

void P2DBody::set_air_friction(f32 new_air_friction)
{
	data.air_friction = new_air_friction;
}

void P2DBody::set_bounce(f32 new_bounce)
{
	data.bounce = new_bounce;
}

void P2DBody::set_shape(const P2DShape& new_shape)
{
	data.shape = new_shape;
	compute_inertia();
}

const P2DShape& P2DBody::get_shape() const
{
	return data.shape;
}

void P2DBody::step(f32 dt)
{
	integrate(dt);

	data.velocity *= 0.999f;
	data.angular_velocity *= 0.999f;
	data.force_accumulator = Vector2();
}

void P2DBody::integrate(f32 dt)
{
	_semi_implicit_euler(dt);
}

void P2DBody::compute_inertia()
{
	data.inertia = data.shape.calculate_inertia(data.mass);
	if(data.inertia > 0)
	{
		data.inv_inertia = 1.f / data.inertia;
	}
	else
	{
		data.inv_inertia = 0.f;
	}
}

void P2DBody::_semi_implicit_euler(f32 dt)
{
	// Linear Velocity?
	const Vector2 acceleration = data.force_accumulator * data.inv_mass;
	data.velocity += acceleration * dt;
	target->translate(data.velocity * dt);

	// Angular Velocity
	if (fixed_rotation == false)
	{
		target->rotate(data.angular_velocity * dt);
	}
}


