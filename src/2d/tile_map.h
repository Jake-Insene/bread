#pragma once
#include "2d/object_2d.h"
#include "resource/tile_set.h"
#include "physics/physics_2d.h"


struct TileMap : Object2D
{
	OBJECT(TileMap, Object2D);

	static void _bind_vtable(TileMap::VTable& vtable);

	struct InternalData
	{
		TileSet* tile_set;

		// Only when there is a tile with a shape.
		Array<Physics2D::BodyID> bodies;
	} data;

	void init(const CreateInfo&);
	void deinit();

	void render();

	void transform_changed();

	/*
	* Set the tile set used to draw the tile map.
	* 
	* @param new_tile_set The new TileSet to draw.
	*/
	void set_tile_set(TileSet* new_tile_set) Function(FunctionNormal);

	/*
	* @return The current TileSet being drawed.
	*/
	TileSet* get_tile_set() const Function(FunctionNormal) { return data.tile_set; }

	Physics2D::BodyID _try_create_physics_body() Function(FunctionInternal);
};