#include "2d/tile_map.h"

#include "graphics/graphics.h"


void TileMap::init(const CreateInfo&)
{
	mark(MARK_RENDER);
}

void TileMap::deinit()
{
	if (data.body_id != Physics2D::BodyID::InvalidID)
	{
		Physics2D::destroy_body(data.body_id);
	}
}

void TileMap::render()
{
	if (data.tile_set == nullptr)
		return;

	Texture2D* tile_map_texture = data.tile_set->get_texture();
	if (tile_map_texture == nullptr)
		return;

	const Vector2 texture_size = Vector2(data.tile_set->get_texture()->get_size());
	const Vector2 tile_size = Vector2(data.tile_set->get_tile_size());

	Transform2D base_transform = get_global_transform();

	Slice<TileSet::Tile> tiles = data.tile_set->get_tiles();
	Slice<TileSet::TileData> tiles_data = data.tile_set->get_tiles_data();
	for (auto& tile : tiles)
	{
		TileSet::TileData& tile_data = tiles_data[tile.data_index];
		Vector2 tile_position = Vector2(tile_data.texture_position);

		Rect2D src_rect{ tile_position, tile_size };

		Transform2D tile_transform = base_transform * Transform2D(
			Vector2(1, 0),
			Vector2(0, 1),
			Vector2(tile_size * Vector2(tile.position))
		);

		Graphics::draw_texture(
			tile_transform, tile_size, src_rect,
			tile_map_texture->texture_id, Color(255, 255, 255, 255),
			RenderCommand::FLAG_BATCH_NONE
		);
	}
}

void TileMap::set_tile_set(TileSet* new_tile_set)
{
	data.tile_set = new_tile_set;
	Slice<TileSet::Tile> tiles = data.tile_set->get_tiles();

	if (data.tile_set == nullptr || tiles.len == 0)
		return;

	_try_create_physics_body();

	Vector2 tile_size = Vector2(data.tile_set->get_tile_size());
	usize shape_count = 0;
	for (auto& tile : tiles)
	{
		if (tile.has_shape_2d == false)
			continue;

		shape_count++;
		Vector2 position = Vector2(tile.position);
		Shape2D tile_shape = Shape2D::make_box(tile_size / 2);

		tile_shape.translate(position * tile_size);
		Physics2D::body_add_shape(data.body_id, tile_shape);
	}

	usize ptr_id = (usize)id.id;
	DebugInfo("TileMap({}): Creating {} shapes", (void*)ptr_id, shape_count);
}


void TileMap::_try_create_physics_body()
{
	if (data.body_id != Physics2D::BodyID::InvalidID)
		return;

	data.body_id = Physics2D::create_body(this);
	Physics2D::body_set_type(data.body_id, Physics2D::STATIC);

}
