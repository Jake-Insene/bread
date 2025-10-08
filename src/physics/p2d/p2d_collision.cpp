#include "physics/p2d/p2d_collision.h"

#include "2d/object_2d.h"
#include "math/values.h"



P2DCollision::SupportPoint P2DCollision::find_support_point(const Vector2& normal_edge, const Vector2& point_on_edge, const P2DShape& other_points)
{
	SupportPoint sp = {};
	f32 current_depthest_penetration = 0.f;

	for (usize i = 0; i < 4; i++)
	{
		const Vector2 vertice = other_points.vertices[i];
		const Vector2 vertice_to_point_edge = vertice - point_on_edge;
		const f32 penetration_depth = vertice_to_point_edge.dot(normal_edge * -1);

		if (penetration_depth > current_depthest_penetration)
		{
			current_depthest_penetration = penetration_depth;
			sp.vertice = vertice;
			sp.penetration_depth = current_depthest_penetration;
			sp.valid = true;
		}
	}

	return sp;
}

CollisionManifold P2DCollision::get_contact_point(const P2DShape& shape_a, const P2DShape& shape_b)
{
	CollisionManifold contact = {};
	f32 minimum_penetration_depth = math::MaxValue<f32>;

	for (usize i = 0; i < 4; i++)
	{
		Vector2 point = shape_a.vertices[i];
		Vector2 normal = shape_a.normals[i];

		SupportPoint support_point = find_support_point(normal, point, shape_b);
		if (!support_point.valid)
			return CollisionManifold();

		if (support_point.penetration_depth < minimum_penetration_depth)
		{
			minimum_penetration_depth = support_point.penetration_depth;
			contact.point = support_point.vertice;
			contact.depth = minimum_penetration_depth;
			contact.normal = normal;
			contact.valid = true;
		}
	}

	return contact;
}

CollisionManifold P2DCollision::polygon_v_polygon(
	const P2DShape& shape_a, const P2DShape& shape_b)
{
	CollisionManifold contact_ab = get_contact_point(shape_a, shape_b);
	if (!contact_ab.valid)
		return CollisionManifold();

	CollisionManifold contact_ba = get_contact_point(shape_b, shape_a);
	if (!contact_ba.valid)
		return CollisionManifold();

	// Getting lowest depth.
	CollisionManifold contact = {};
	if (contact_ab.depth < contact_ba.depth)
	{
		contact = contact_ab;
	}
	else
	{
		contact = contact_ba;
		contact.normal *= -1;
	}

	return contact;
}

void P2DCollision::positional_correction(CollisionManifold& manifold, P2DBody& body_a, P2DBody& body_b)
{
	const f32 correction_percentage = 1.0f;

	f32 inv_mass_a = body_a.get_inv_mass();
	f32 inv_mass_b = body_b.get_inv_mass();
	f32 inv_mass_sum = inv_mass_a + inv_mass_b;

	if (inv_mass_sum > 0)
	{
		inv_mass_sum = 1.f / inv_mass_sum;
	}
	else if (body_a.type == Physics2D::KINEMATIC)
	{
		inv_mass_sum = 1.f;
		inv_mass_a = 1.f;
	}

	const f32 amount_to_correct = manifold.depth * inv_mass_sum * correction_percentage;
	const Vector2 correction_vector = manifold.normal * amount_to_correct;

	const Vector2 body_a_movement = correction_vector * inv_mass_a * -1;
	const Vector2 body_b_movement = correction_vector * inv_mass_b;

	body_a.target->translate(body_a_movement);
	body_b.target->translate(body_b_movement);
}

void P2DCollision::resolve_collision(CollisionManifold& manifold, P2DBody& body_a, P2DBody& body_b)
{
	if (body_a.type != Physics2D::DYNAMIC
		&& body_b.type != Physics2D::DYNAMIC)
		return;

	const Vector2 local_centroid_a = body_a.get_shape().get_centroid();
	const Vector2 world_centroid_a = body_a.target->get_global_transform() * local_centroid_a;
	const Vector2 penetration_to_centeroid_a = manifold.point - world_centroid_a;

	const Vector2 local_centroid_b = body_b.get_shape().get_centroid();
	const Vector2 world_centroid_b = body_b.target->get_global_transform() * local_centroid_b;
	const Vector2 penetration_to_centeroid_b = manifold.point - world_centroid_b;

	const Vector2 angular_velocity_penetration_centeroid_a = Vector2(
		-1 * body_a.get_angular_velocity() * penetration_to_centeroid_a.y,
		body_a.get_angular_velocity() * penetration_to_centeroid_a.x
	);

	const Vector2 angular_velocity_penetration_centeroid_b = Vector2(
		-1 * body_b.get_angular_velocity() * penetration_to_centeroid_b.y,
		body_b.get_angular_velocity() * penetration_to_centeroid_b.x
	);

	const Vector2 velocity_a = body_a.get_velocity() + angular_velocity_penetration_centeroid_a;
	const Vector2 velocity_b = body_b.get_velocity() + angular_velocity_penetration_centeroid_b;
	const Vector2 relative_velocity = velocity_b - velocity_a;
	const f32 relative_velocity_along_normal = relative_velocity.dot(manifold.normal);

	if (relative_velocity_along_normal > 0.f)
		return;

	f32 inv_bounce_sum = body_a.get_bounce() + body_b.get_bounce();
	if (inv_bounce_sum > 0)
	{
		inv_bounce_sum = 1.f / inv_bounce_sum;
	}
	const f32 e = (2 * body_a.get_bounce() * body_b.get_bounce()) * inv_bounce_sum;
	const f32 p_to_centeroid_cross_normal_a = Vector2::cross(penetration_to_centeroid_a, manifold.normal);
	const f32 p_to_centeroid_cross_normal_b = Vector2::cross(penetration_to_centeroid_b, manifold.normal);
	const f32 inv_mass_sum = body_a.get_inv_mass() + body_b.get_inv_mass();

	const f32 inv_inertia_a = body_a.get_inv_inertia();
	const f32 inv_inertia_b = body_b.get_inv_inertia();

	const f32 cross_n_sum = p_to_centeroid_cross_normal_a * p_to_centeroid_cross_normal_a * inv_inertia_a
							+ p_to_centeroid_cross_normal_b * p_to_centeroid_cross_normal_b * inv_inertia_b;

	f32 j = -(1.f + e) * relative_velocity_along_normal;
	j /= (inv_mass_sum + cross_n_sum);

	const Vector2 impulse = manifold.normal * j;
	const Vector2 impulse_body_a = impulse * body_a.get_inv_mass() * -1;
	const Vector2 impulse_body_b = impulse * body_b.get_inv_mass();
	body_a.set_velocity(body_a.get_velocity() + impulse_body_a);
	body_b.set_velocity(body_b.get_velocity() + impulse_body_b);

	body_a.set_angular_velocity(
		body_a.get_angular_velocity() + -p_to_centeroid_cross_normal_a * j * inv_inertia_a
	);
	body_b.set_angular_velocity(
		body_b.get_angular_velocity() + p_to_centeroid_cross_normal_b * j * inv_inertia_b
	);

	// Friction
	const f32 friction = (body_a.get_friction() + body_b.get_friction()) * 0.5f;
	if (friction > 0)
	{
		const Vector2 tangent = Vector2(-manifold.normal.y, manifold.normal.x);
		const f32 relative_vel_tangent = relative_velocity.dot(tangent);

		const f32 max_friction = friction * abs(j);
		f32 friction_impulse;
		if (abs(relative_vel_tangent) < max_friction)
			friction_impulse = relative_vel_tangent;
		else
			friction_impulse = max_friction * (relative_vel_tangent > 0 ? -1 : 1);

		const Vector2 friction_vec = tangent * friction_impulse;
		const Vector2 friction_body_a = friction_vec * body_a.get_inv_mass() * -1;
		const Vector2 friction_body_b = friction_vec * body_b.get_inv_mass();
		body_a.set_velocity(body_a.get_velocity() + friction_body_a);
		body_b.set_velocity(body_b.get_velocity() + friction_body_b);

		const f32 p_to_centeroid_cross_tangent_a = Vector2::cross(penetration_to_centeroid_a, tangent);
		const f32 p_to_centeroid_cross_tangent_b = Vector2::cross(penetration_to_centeroid_b, tangent);
		body_a.set_angular_velocity(
			body_a.get_angular_velocity() + -p_to_centeroid_cross_tangent_a * friction_impulse * inv_inertia_a
		);
		body_b.set_angular_velocity(
			body_b.get_angular_velocity() + p_to_centeroid_cross_tangent_b * friction_impulse * inv_inertia_b
		);
	}
}
