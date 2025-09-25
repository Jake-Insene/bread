#pragma once
#include "physics/p2d/p2d_shape.h"


struct P2DCollision
{
	struct CollisionManifold
	{
		Vector2 normal;
		Vector2 point;
		f32 depth;
		bool valid;
	};

	struct SupportPoint
	{
		Vector2 vertice;
		f32 penetration_depth;
		bool valid;
	};

	static SupportPoint find_support_point(const Vector2& normal_edge, const Vector2& point_on_edge, const P2DShape& other_points);

	static CollisionManifold get_contact_point(const P2DShape& shape_a, const P2DShape& shape_b);

	static CollisionManifold polygon_v_polygon(const P2DShape& shape_a, const P2DShape& shape_b);
};

