#pragma once
#include "collections/array.h"
#include "resource/texture.h"


// A group of tiles
struct TileSet : Resource
{
	RESOURCE(RESOURCE_TILE_SET, ResourceFlags::LoadFromAssets, ResourceExtensions(""))
	
	struct Tile
	{
		Vector2I position;
		i32 data_index;
		bool has_shape_2d;
	};

	struct TileData
	{
		Vector2I texture_position;
	};

	struct InternalData
	{
		Vector2I tile_size;
		Texture2D* texture;

		Array<Tile> tiles;
		Array<TileData> tiles_data;
	} data;

	void init(const ResourceCreateInfo& info);
	void destroy();

	void set_tile_size(Vector2I new_tile_size) { data.tile_size = new_tile_size; }
	Vector2I get_tile_size() const { return data.tile_size; }

	void set_texture(Texture2D* new_texture);
	Texture2D* get_texture() const { return data.texture; }

	void add_tiles(Slice<Tile> new_tiles);
	void set_tiles(Slice<Tile> new_tiles);
	const Slice<Tile> get_tiles() const { return data.tiles.slice(); }

	void add_tiles_data(Slice<TileData> new_tiles_data);
	void set_tiles_data(Slice<TileData> new_tiles_data);
	const Slice<TileData> get_tiles_data() const { return data.tiles_data.slice(); }
};