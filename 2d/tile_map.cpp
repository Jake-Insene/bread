#include "2d/tile_map.h"

#include "graphics/graphics.h"


void TileMap::init(const CreateInfo&)
{
	mark(MARK_RENDER);
}

void TileMap::deinit()
{
}

void TileMap::render()
{
	if (data.tile_set && data.tile_set->texture)
	{
		const Vector2 extent = Vector2(data.tile_set->texture->get_size());
		const Vector2 tile_size = Vector2(data.tile_set->tile_size);

		Transform2D base_transform = get_global_transform();

		for (auto& tile : data.tile_set->tiles)
		{
			TileSet::TileData& tile_data = data.tile_set->tiles_data[tile.data_index];
			Vector2 tile_position = Vector2(tile_data.texture_position);

			Rect2D src_rect{ tile_position, tile_position + tile_size};

			Transform2D tile_transform = base_transform * Transform2D(
				Vector2(1, 0),
				Vector2(0, 1),
				Vector2(tile_size * Vector2(tile.position))
			);

			Graphics2D::draw_texture(
				tile_transform, extent, tile_size, src_rect, 
				data.tile_set->texture->texture_id, Color(255, 255, 255, 255),
				RenderCommand::FLAG_SPRITE_NONE
			);
		}
	}
}

void TileMap::set_tile_set(TileSet* new_tile_set)
{
	data.tile_set = new_tile_set;
}

