#include "io/tile_set.h"


void TileSet::destroy()
{
	Resource::destroy();
	tiles.destroy();
	tiles_data.destroy();
}

void TileSet::add_tiles(Slice<Tile> new_tiles)
{
	tiles.add_slice(new_tiles);
}

void TileSet::set_tiles(Slice<Tile> new_tiles)
{
	tiles.replace(new_tiles);
}

void TileSet::add_tiles_data(Slice<TileData> new_tiles_data)
{
	tiles_data.add_slice(new_tiles_data);
}

void TileSet::set_tiles_data(Slice<TileData> new_tiles_data)
{
	tiles_data.replace(new_tiles_data);
}
