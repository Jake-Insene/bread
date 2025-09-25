#pragma once
#include "math/transform_2d.h"
#include "physics/shape_2d.h"


struct [[nodiscard]] P2DShape
{
	Vector2 vertices[4];
	Vector2 normals[4];
	f32 area;
	Vector2 centroid;

	static P2DShape from_shape_2d(const Shape2D& shape);

	void apply_transform(const Transform2D& transform);
	void translate(const Vector2& translation);
	void rotate(const f32 r);

	[[nodiscard]] f32 get_area() const;
	[[nodiscard]] Vector2 get_centroid() const;

	Shape2D to_shape_2d() const;

	void _calc_normals();
	void _calc_area();
	void _calc_centroid();
};