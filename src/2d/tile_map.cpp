#include "2d/tile_map.h"

#include "log/log.h"


void TileMap::_bind_vtable(TileMap::VTable& vtable)
{
	BindVTable(vtable, transform_changed, &TileMap::transform_changed);
}

void TileMap::init(const CreateInfo&)
{
	data.bodies = Array<Physics2D::BodyID>::with_size(allocator, 4);
}

void TileMap::deinit()
{
	data.bodies.iter().for_each(Physics2D::destroy_body);
	data.bodies.destroy();
}

void TileMap::render()
{
	if (data.tile_set == nullptr)
		return;

	Texture2D* tile_map_texture = data.tile_set->get_texture();
	if (tile_map_texture == nullptr)
		return;

	const Vector2 tile_size = Vector2(data.tile_set->get_tile_size());

	Transform2D base_transform = get_global_transform();

	Slice<TileSet::Tile> tiles = data.tile_set->get_tiles();
	Slice<TileSet::TileData> tiles_data = data.tile_set->get_tiles_data();
	for (auto& tile : tiles)
	{
		TileSet::TileData& tile_data = tiles_data[tile.data_index];
		Vector2 tile_position = Vector2(tile_data.texture_position);

		Rect2D src_rect = Rect2D(tile_position, tile_size);

		Transform2D tile_transform = base_transform * Transform2D(
			Vector2(1, 0),
			Vector2(0, 1),
			Vector2(tile_size * Vector2(tile.position))
		);

		Rect2D rect = Rect2D(
			Vector2(0, 0), tile_size
		);

		draw_sprite(
			tile_transform, tile_map_texture->texture_id, rect,
			src_rect, Color(255, 255, 255, 255), 0
		);
	}
}

void TileMap::transform_changed()
{
	data.bodies.iter().for_each([&] (Physics2D::BodyID body_id)
	{ 
		Physics2D::body_set_transform(body_id, get_global_transform());
	});
}

void TileMap::set_tile_set(TileSet* new_tile_set)
{
	data.tile_set = new_tile_set;
	Slice<TileSet::Tile> tiles = data.tile_set->get_tiles();

	if (data.tile_set == nullptr || tiles.len == 0)
		return;


	Vector2 tile_size = Vector2(data.tile_set->get_tile_size());
	usize shape_count = 0;
	for (auto& tile : tiles)
	{
		if (tile.has_shape_2d == false)
			continue;

		Physics2D::BodyID body_id = _try_create_physics_body();

		shape_count++;
		Vector2 position = Vector2(tile.position);
		Shape2D tile_shape = Shape2D::make_box(tile_size / 2);
		// Centering...
		tile_shape.translate(Vector2(tile_size.x / 2.f, -tile_size.y / 2.f));

		tile_shape.translate(position * tile_size);
		Physics2D::body_set_shape(body_id, tile_shape);
		Physics2D::body_set_transform(body_id, get_global_transform());
	}

	Log::debug("TileMap({}): Creating {} shapes", (void*)usize(id.id), shape_count);
}


Physics2D::BodyID TileMap::_try_create_physics_body()
{
	Physics2D::BodyID body_id = Physics2D::create_body(this);
	Physics2D::body_set_type(body_id, Physics2D::STATIC);
	(void)data.bodies.add(body_id);
	return body_id;
}
