#include "graphics/viewport.h"


Viewport Viewport::create_from_render_target(const mem::Allocator& allocator, RenderTarget rt)
{
	Viewport vp = Viewport
	{
		.allocator = allocator,
		.items = FreeList<RenderItem, RenderItemID>::with_size(allocator, 16),
		.rt = rt,
		.viewport_size = Vector2I(),
		.clear_color = Color(0, 0, 0, 255),
		.must_sync = false,
	};

	for (auto& layer : vp.order_table.layers)
	{
		layer.items = Array<RenderItemID>::with_size(allocator, 16);
	}

	return vp;
}


void Viewport::destroy()
{
	rt.destroy();

	for (auto& layer : order_table.layers)
	{
		layer.items.destroy();
	}

	items.destroy();
}

void Viewport::reset_commands()
{
	for (auto& layer : order_table.layers)
	{
		for (auto& item_id : layer.items)
		{
			RenderItem& item = items.get(item_id);
			item.reset();
		}
	}
}

void Viewport::set_scene_transform(const Transform2D& transform)
{
	scene_transform = transform;
}

void Viewport::set_size(const Vector2I& new_size)
{
	viewport_size = new_size;
	must_sync = true;
	rt.set_size(new_size);
}

Vector2I Viewport::get_size() const
{
	return rt.get_size();
}

RenderItemID Viewport::create_item(ViewportLayerMask layers)
{
	RenderItemID new_item = items.add(RenderItem());
	RenderItem& item = items.get(new_item);
	
	item.self = new_item;
	item.command_buffer = allocator.alloc(DefaultCommandBufferSize, alignof(RenderItem::Command));
	item_set_layers(new_item, layers);

	return new_item;
}

void Viewport::destroy_item(RenderItemID render_item_id)
{
	RenderItem& item = items.get(render_item_id);
	allocator.free(item.command_buffer);

	item_set_layers(render_item_id, ViewportLayerMask(0));

	items.remove(render_item_id);
}

void Viewport::item_set_layers(RenderItemID render_item_id, ViewportLayerMask layers)
{
	RenderItem& item = items.get(render_item_id);
	
	ViewportLayerMask old_layers = item.layers;
	item.layers = layers;

	for (usize layer = 0; layer < VIEWPORT_LAYER_COUNT; layer++)
	{
		ViewportLayerMask layer_mask = ViewportLayerMask(1 << layer);
		ViewportLayer& vp_layer = order_table.layers[layer];

		if ((item.layers & layer_mask) && (old_layers & layer_mask))
		{
			continue;
		}
		
		if (item.layers & layer_mask)
		{
			(void)vp_layer.items.add(render_item_id);
		}
		else if(old_layers & layer_mask)
		{
			vp_layer.items.remove_equal(render_item_id);
		}
	}
}

Viewport::ViewportLayerMask Viewport::item_get_layers(RenderItemID render_item_id)
{
	RenderItem& item = items.get(render_item_id);
	return item.layers;
}

void Viewport::render_item_draw_sprite(RenderItemID render_item_id, const Transform2D& transform, 
	TextureID texture, const Rect2D& rect, const Rect2D& src_rect, Color mod_color, RenderFlags flags)
{
	RenderItem& item = items.get(render_item_id);
	auto sprite = item.alloc<RenderItem::CommandSprite>();
	sprite->type = RenderItem::CMD_SPRITE;
	sprite->transform = transform;
	sprite->texture = texture;
	sprite->rect = rect;
	sprite->src_rect = src_rect;
	sprite->mod_color = mod_color;
	sprite->flags = flags;
}

void Viewport::render_item_draw_ui_sprite(RenderItemID render_item_id, const Transform2D& transform, TextureID texture,
	const Rect2D& rect, const Rect2D& src_rect, Color mod_color, RenderFlags flags)
{
	RenderItem& item = items.get(render_item_id);
	auto ui_sprite = item.alloc<RenderItem::CommandUISprite>();
	ui_sprite->type = RenderItem::CMD_UI_SPRITE;
	ui_sprite->transform = transform;
	ui_sprite->texture = texture;
	ui_sprite->rect = rect;
	ui_sprite->src_rect = src_rect;
	ui_sprite->mod_color = mod_color;
	ui_sprite->flags = flags;
}

void Viewport::render_item_draw_rect(RenderItemID render_item_id, const Transform2D& transform,
	const Rect2D& dest_rect, Color color)
{
	RenderItem& item = items.get(render_item_id);
	auto rect = item.alloc<RenderItem::CommandRect>();
	rect->type = RenderItem::CMD_RECT;
	rect->transform = transform;
	rect->rect = dest_rect;
	rect->color = color;
}

