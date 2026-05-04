#include "physics/p2d/p2d_body.h"


void P2DBody::init(const mem::Allocator& allocator, Physics2D::BodyID id, Opaque* ud)
{
	self = id;
	user_data = ud;
	type = Physics2D::DYNAMIC;
	residence_mask = Physics2D::CollisionMask(Physics2D::DEFAULT_COLLISION_MASK);
    collision_mask = Physics2D::CollisionMask(Physics2D::DEFAULT_COLLISION_MASK);

	data.force_accumulator = Vector2();
	data.torque_accumulator = 0.F;
	data.angular_velocity = 0.F;

	shape.init(allocator);
	data.shape_transformed.init(allocator);
    set_mass(1.F);
    set_friction(0.1F);
    set_air_friction(0.001F);
    set_restitution(0.5F);
    set_velocity(Vector2());
    set_angular_velocity(0.F);
    set_transform(Transform2D());
    
	_compute_inertia();

    tiles_on = Array<PhysicsTileCoord>::with_size(allocator, 4);
}

void P2DBody::destroy()
{
	shape.destroy();
	data.shape_transformed.destroy();
    tiles_on.destroy();
}

void P2DBody::apply_force(const Vector2& force, const Vector2& point)
{
    const Vector2 direction = point - shape.centroid;
	add_force(force);

	data.torque_accumulator += Vector2::cross(direction, force);
}

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

void P2DBody::add_velocity(const Vector2& vel)
{
	data.velocity += vel;
}

void P2DBody::set_angular_velocity(f32 new_angular_velocity)
{
	if(fixed_rotation)
	{
		return;
	}
	data.angular_velocity = new_angular_velocity;
}

f32 P2DBody::get_angular_velocity() const
{
	return data.angular_velocity;
}

void P2DBody::add_angular_velocity(f32 ang_vel)
{
	if(fixed_rotation)
	{
		return;
	}
	data.angular_velocity += ang_vel;
}

void P2DBody::set_mass(f32 new_mass)
{
	if (new_mass > 0)
	{
		data.mass = new_mass;
		data.inv_mass = 1.F / new_mass;
	}
	else
	{
		data.mass = 0.F;
		data.inv_mass = 0.F;
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

void P2DBody::set_restitution(f32 new_restitution)
{
	data.restitution = new_restitution;
}

void P2DBody::set_shape_from_2d(const Shape2D& new_shape)
{
	shape.set_from_shape_2d(new_shape);
	data.shape_transformed.set_from_shape_2d(new_shape);
	_compute_inertia();
}


void P2DBody::set_transform(const Transform2D& new_transform)
{
	data.transform = new_transform;
	
	data.shape_transformed.set_from_shape(shape);
	data.shape_transformed.apply_transform(new_transform);
}

void P2DBody::step(f32 dt)
{
	integrate(dt);

	data.velocity *= (1 - (data.air_friction * dt));
	data.angular_velocity *= (1 - (data.air_friction * dt));

	data.force_accumulator = Vector2();
	
	// Simple sleep based on velocity
	const f32 rest_threshold = 0.001F;
	const f32 rest_threshold_square = rest_threshold * rest_threshold;
	if (data.velocity.dot(data.velocity) < rest_threshold_square)
	{
		data.velocity = Vector2();
	}
	if (math::abs(data.angular_velocity) < rest_threshold)
	{
		data.angular_velocity = 0.F;
	}
}

void P2DBody::integrate(f32 dt)
{
	_semi_implicit_euler(dt);
}

void P2DBody::_compute_inertia()
{
	data.inertia = shape.calculate_inertia(data.mass);
	if(data.inertia > 0)
	{
		data.inv_inertia = 1.F / data.inertia;
	}
	else
	{
		data.inv_inertia = 0.F;
	}
}

void P2DBody::_semi_implicit_euler(f32 dt)
{
	// Linear Velocity?
	const Vector2 acceleration = data.force_accumulator * data.inv_mass;
	data.velocity += acceleration * dt;
	data.transform.translate(data.velocity * dt);

	// Angular acceleration
	const f32 angular_acceleration = data.torque_accumulator * data.inv_inertia;
	data.angular_velocity += angular_acceleration;

	// Angular Velocity
	if (!fixed_rotation)
	{
		data.transform.rotate(data.angular_velocity * dt);
	}

	data.shape_transformed.set_from_shape(shape);
	data.shape_transformed.apply_transform(data.transform);
}


