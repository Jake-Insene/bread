#pragma once
#include "2d/object_2d.h"
#include "resource/tile_set.h"
#include "physics/physics_2d.h"


struct TileMap : Object2D
{
	OBJECT(TileMap, Object2D);

	struct InternalData
	{
		TileSet* tile_set;

		// Only when there is a tile with a shape.
		Physics2D::BodyID body_id;
	} data;

	void init(const CreateInfo&);
	void deinit();

	void render();

	void set_tile_set(TileSet* new_tile_set);
	TileSet* get_tile_set() const { return data.tile_set; }

	void _try_create_physics_body();
};