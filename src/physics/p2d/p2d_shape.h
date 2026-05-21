#pragma once
#include "collections/array.h"
#include "math/transform_2d.h"
#include "math/aabb.h"
#include "physics/shape_2d.h"



struct [[nodiscard]] P2DShape
{
	Array<Vector2> vertices;
	Array<Vector2> normals;
	f32 area;
	Vector2 centroid;
	AABB aabb;

	void init(Mem::Allocator* allocator);
	void destroy();

	P2DShape copy() const;
	void set_from_shape_2d(const Shape2D& shape_2d);
	void set_from_shape(const P2DShape& other_shape);

	void apply_transform(const Transform2D& transform);
	void translate(const Vector2& translation);
	void rotate(const f32 r);
	
	[[nodiscard]] f32 get_area() const;
	[[nodiscard]] Vector2 get_centroid() const;

	f32 calculate_inertia(f32 mass) const;

	Shape2D to_shape_2d() const;

	void _calc_aabb();
	void _calc_normals();
	void _calc_area();
	void _calc_centroid();
};