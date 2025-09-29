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

P2DCollision::CollisionManifold P2DCollision::get_contact_point(const P2DShape& shape_a, const P2DShape& shape_b)
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

P2DCollision::CollisionManifold P2DCollision::polygon_v_polygon(
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
	const f32 correction_percentage = 1.f;

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

	if (body_a.type != Physics2D::STATIC)
	{
		body_a.target->translate(body_a_movement);
	}
	if (body_b.type != Physics2D::STATIC)
	{
		body_b.target->translate(body_b_movement);
	}

}

void P2DCollision::resolve_collision(CollisionManifold& manifold, P2DBody& body_a, P2DBody& body_b)
{
	// Linear impulse
	const Vector2 relative_velocity = body_b.get_velocity() - body_a.get_velocity();
	const f32 relative_velocity_along_normal = relative_velocity.dot(manifold.normal);
	if (relative_velocity_along_normal > 0.f)
	{
		return;
	}

	if (body_a.type != Physics2D::DYNAMIC && body_b.type != Physics2D::DYNAMIC)
	{
		return;
	}

	const f32 inv_mass_sum = body_a.get_inv_mass() + body_b.get_inv_mass();

	//const f32 e = math::min(body_a.get_bounce(), body_b.get_bounce());
	f32 inv_bounce_sum = body_a.get_bounce() + body_b.get_bounce();
	if (inv_bounce_sum > 0)
	{
		inv_bounce_sum = 1.f / inv_bounce_sum;
	}
	const f32 e = (2*body_a.get_bounce()*body_b.get_bounce()) * inv_bounce_sum;

	f32 j = -(1.f + e) * relative_velocity_along_normal;
	j /= inv_mass_sum;

	const Vector2 impulse = manifold.normal * j;
	const Vector2 impulse_body_a = impulse * body_a.get_inv_mass() * -1;
	const Vector2 impulse_body_b = impulse * body_b.get_inv_mass();
	body_a.set_velocity(body_a.get_velocity() + impulse_body_a);
	body_b.set_velocity(body_b.get_velocity() + impulse_body_b);
}
