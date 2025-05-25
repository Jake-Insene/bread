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

			Graphics::add_cmd(
				RenderCommand
				{
					.type = RenderCommand::DRAW_SPRITE,
					.sprite =
					{
						.transform = tile_transform,
						.texture_extent = extent,
						.dest_extent = tile_size,
						.src_rect = src_rect,
						.texture = data.tile_set->texture->texture_id,
						.color = Color(255, 255, 255, 255),
						.flags = RenderCommand::FLAG_NONE,
					}
				}
			);
		}
	}
}

void TileMap::set_tile_set(TileSet* new_tile_set)
{
	data.tile_set = new_tile_set;
}

