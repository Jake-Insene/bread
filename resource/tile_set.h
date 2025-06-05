#pragma once
#include "collections/array.h"
#include "resource/texture.h"


// A group of tiles
struct TileSet : Resource
{
	RESOURCE(RESOURCE_TILE_SET, .LoadFromAssets = false, .Extensions = "");
	
	struct Tile
	{
		Vector2I position;
		i32 data_index;
	};

	struct TileData
	{
		Vector2I texture_position;
	};

	Array<Tile> tiles;
	Array<TileData> tiles_data;
	Texture2D* texture;

	Vector2I tile_size;

	void init();
	void destroy();

	void add_tiles(Slice<Tile> new_tiles);
	void set_tiles(Slice<Tile> new_tiles);

	void add_tiles_data(Slice<TileData> new_tiles_data);
	void set_tiles_data(Slice<TileData> new_tiles_data);
};