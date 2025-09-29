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
}

const P2DShape& P2DBody::get_shape() const
{
	return data.shape;
}

void P2DBody::step(f32 dt)
{
	integrate(dt);

	data.force_accumulator = Vector2();
	data.velocity *= 0.999f;
}

void P2DBody::integrate(f32 dt)
{
	_semi_implicit_euler(dt);
	//runge_kutta4(dt);
}

void P2DBody::_semi_implicit_euler(f32 dt)
{
	const Vector2 acceleration = data.force_accumulator * data.inv_mass;
	data.velocity += acceleration * dt;
	target->translate(data.velocity * dt);
}

void P2DBody::runge_kutta4(f32 dt)
{
	Vector2 k1, k2, k3, k4;

	const auto compute_acceleration = [] (const Vector2& force, f32 inv_mass) { return force * inv_mass; };

	// compute k1
	Vector2 acceleration = compute_acceleration(data.force_accumulator, data.inv_mass);
	k1 = acceleration * dt;

	// compute k2
	Vector2 temp_force = data.force_accumulator + (k1 * 0.5f);
	acceleration = compute_acceleration(temp_force, data.inv_mass);
	k2 = acceleration * dt;

	// compute k3
	temp_force = data.force_accumulator + (k2 * 0.5f);
	acceleration = compute_acceleration(temp_force, data.inv_mass);
	k3 = acceleration * dt;

	// compute k4
	temp_force = data.force_accumulator + (k3 * 0.5f);
	acceleration = compute_acceleration(temp_force, data.inv_mass);
	k4 = acceleration * dt;

	// combine to get the new velocity
	// ((k2 x 2) + k1) + (k3 x 2) + k4) / 6
	// (k1 + 2xk2 + 2xk3 + k4) / 6
	Vector2 dv = (
		(k1 + (k2 * 2.f) + (k3 * 2.f) + k4)
	);
	dv *= (1.f / 6.f);
	data.velocity += dv;

	const Vector2& dp = data.velocity * dt;
	target->translate(dp);
}

