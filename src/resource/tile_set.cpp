#include "resource/tile_set.h"


void TileSet::init(const ResourceCreateInfo& info)
{
	Resource::init(info);
	data.tiles = Array<TileSet::Tile>::with_allocator(allocator);
	data.tiles_data = Array<TileSet::TileData>::with_allocator(allocator);
}

void TileSet::destroy()
{
	data.tiles.destroy();
	data.tiles_data.destroy();
	Resource::destroy();
}

void TileSet::set_texture(Texture2D* new_texture)
{
	data.texture = new_texture;
}

void TileSet::add_tiles(Slice<Tile> new_tiles)
{
	data.tiles.add_slice(new_tiles);
}

void TileSet::set_tiles(Slice<Tile> new_tiles)
{
	data.tiles.replace(new_tiles);
}

void TileSet::add_tiles_data(Slice<TileData> new_tiles_data)
{
	data.tiles_data.add_slice(new_tiles_data);
}

void TileSet::set_tiles_data(Slice<TileData> new_tiles_data)
{
	data.tiles_data.replace(new_tiles_data);
}
