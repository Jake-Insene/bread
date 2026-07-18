#pragma once
#include "math/vec2.h"


/*
* Axis Aligned Bounding Box
*/
struct [[nodiscard]] AABB
{
	Vector2 min;
	Vector2 max;

	[[nodiscard]] constexpr bool intersecs(const AABB& aabb) const
	{
		return !(aabb.min.x > max.x
			|| aabb.min.y > max.y 
			|| min.x > aabb.max.x
			|| min.y > aabb.max.y);
	}


};