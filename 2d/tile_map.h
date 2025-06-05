#pragma once
#include "2d/object_2d.h"
#include "resource/tile_set.h"


struct TileMap : Object2D
{
	OBJECT(TileMap, Object2D);

	// As everything in a struct is public we need to hide data
	// that should not be modified/access directly, this also
	// resolve some namespace problems.
	struct InternalData
	{
		TileSet* tile_set;
	} data;

	void init(const CreateInfo&);
	void deinit();

	void render();

	void set_tile_set(TileSet* new_tile_set);
	TileSet* get_tile_set() const { return data.tile_set; }
};