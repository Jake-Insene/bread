#include "physics/p2d/p2d_area.h"


void P2DArea::set_shape(const P2DShape& new_shape)
{
	data.shape = new_shape;
	data.shape_transformed = new_shape;
}

void P2DArea::set_transform(const Transform2D& new_transform)
{
	data.transform = new_transform;
	data.shape_transformed = data.shape;
	data.shape_transformed.apply_transform(new_transform);
}