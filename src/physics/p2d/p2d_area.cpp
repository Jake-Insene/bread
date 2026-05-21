#include "physics/p2d/p2d_area.h"




void P2DArea::init(Mem::Allocator* allocator, Physics2D::AreaID id, Opaque* ud)
{
	self = id;
	user_data = ud;
    residence_mask = Physics2D::CollisionMask(Physics2D::DEFAULT_COLLISION_MASK);

	shape.init(allocator);
	data.shape_transformed.init(allocator);
    bodies_inside = HashMap<Physics2D::BodyID, P2DArea::BodyInArea>::with_size(allocator, 4);
    tiles_on = Array<PhysicsTileCoord>::with_size(allocator, 4);
    check_counter = 0;
}

void P2DArea::destroy()
{
	shape.destroy();
	data.shape_transformed.destroy();
	bodies_inside.destroy();	
	tiles_on.destroy();	
}

void P2DArea::set_shape_from_2d(const Shape2D& new_shape)
{
	shape.set_from_shape_2d(new_shape);
	data.shape_transformed.set_from_shape_2d(new_shape);
}


void P2DArea::set_transform(const Transform2D& new_transform)
{
	data.transform = new_transform;
	
	data.shape_transformed.set_from_shape(shape);
	data.shape_transformed.apply_transform(data.transform);
}