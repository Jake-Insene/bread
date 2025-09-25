#include "physics/p2d/p2d_collision.h"

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