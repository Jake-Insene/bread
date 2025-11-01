#include "graphics/render_manager.h"

#include "graphics/graphics.h"

// Render Item

void RenderManager::RenderItem::destroy()
{
	childs.destroy();
	allocator.free(command_buffer);
}

RenderManager::RenderItem::Command* RenderManager::RenderItem::alloc_base(usize command_size)
{
	if (offset + command_size > command_buffer.len)
	{
		auto new_command_buffer = allocator.alloc(command_buffer.len * 2, alignof(RenderItem::Command));
		mem::copy(new_command_buffer, command_buffer);
		allocator.free(command_buffer);
		command_buffer = new_command_buffer;
	}

	DebugAssert(offset < command_buffer.len, "command buffer full!");
	Command* new_cmd = reinterpret_cast<Command*>(command_buffer.add(offset).ptr());
	allocator.construct(new_cmd);
	usize cmd_offset = usize(new_cmd) - usize(begin());
	offset += command_size;
	new_cmd->next = offset;

	if (last_element == usize(-1))
	{
		last_element = cmd_offset;
	}
	else
	{
		get_command_at(last_element)->next = cmd_offset;
		last_element = cmd_offset;
	}
	return new_cmd;
}

void RenderManager::RenderItem::reset()
{
	offset = 0;
	last_element = usize(-1);
}


// RenderManager


void RenderManager::initialize(const mem::Allocator& allocator)
{
	data.allocator = allocator;
	data.items = FreeList<RenderItem, RenderItemID>::with_size(allocator, 16);

	for (auto& layer : data.order_table.layers)
	{
		layer.items = Array<RenderItemID>::with_size(allocator, 16);
	}
}

void RenderManager::shutdown()
{
	for (RenderLayer& layer : data.order_table.layers)
	{
		for (RenderItemID item_id : layer.items.iter())
		{
			RenderItem& item = data.items.get(item_id);
			if (item.material != MaterialID())
			{
				Graphics::destroy_material(item.material);
			}
		}
		layer.items.destroy();
	}

	data.items.destroy();
}

RenderManager::RenderItem& RenderManager::_get_item(RenderItemID render_item_id)
{
	return data.items.get(render_item_id);
}

void RenderManager::reset_commands()
{
	for (RenderLayer& layer : data.order_table.layers)
	{
		for (RenderItemID item_id : layer.items.iter())
		{
			RenderItem& item = data.items.get(item_id);
			item.reset();
		}
	}
}

RenderItemID RenderManager::create_item()
{
	RenderItemID new_item = data.items.add(RenderItem());
	RenderItem& item = _get_item(new_item);

	item.self = new_item;
	item.allocator = get_allocator();
	item.command_buffer = get_allocator().alloc(
		DefaultCommandBufferSize, alignof(RenderItem::Command)
	);
	item.childs = Array<RenderItemID>::with_size(
		get_allocator(), DefaultChildSize
	);

	item_set_layers(new_item, RENDER_LAYER_DEFAULT);
	return new_item;
}

void RenderManager::destroy_item(RenderItemID render_item_id)
{
	RenderItem& item = _get_item(render_item_id);
	item_set_layers(render_item_id, RenderLayerMask(0));
	item.destroy();

	data.items.remove(render_item_id);
}

void RenderManager::item_set_parent(RenderItemID render_item_id, RenderItemID parent_id)
{
	RenderItem& item = _get_item(render_item_id);
	item.parent = parent_id;

	RenderItem& parent = _get_item(parent_id);
	(void)parent.childs.add(render_item_id);
}

void RenderManager::item_set_material(RenderItemID render_item_id, MaterialID material)
{
	RenderItem& item = _get_item(render_item_id);
	item.material = material;
}

MaterialID RenderManager::item_get_material(RenderItemID render_item_id)
{
	RenderItem& item = _get_item(render_item_id);
	return item.material;
}

void RenderManager::item_set_layers(RenderItemID render_item_id, RenderLayerMask layers)
{
	RenderItem& item = _get_item(render_item_id);

	RenderLayerMask old_layers = item.layers;
	item.layers = layers;

	for (usize layer = 0; layer < RENDER_LAYER_COUNT; layer++)
	{
		RenderLayerMask layer_mask = RenderLayerMask(1 << layer);
		RenderLayer& render_layer = get_order_table().layers[layer];

		if ((item.layers & layer_mask) && (old_layers & layer_mask))
		{
			continue;
		}

		if (item.layers & layer_mask)
		{
			(void)render_layer.items.add(render_item_id);
		}
		else if (old_layers & layer_mask)
		{
			render_layer.items.remove(render_item_id);
		}
	}
}

RenderManager::RenderLayerMask RenderManager::item_get_layers(RenderItemID render_item_id)
{
	RenderItem& item = _get_item(render_item_id);
	return item.layers;
}

void RenderManager::render_item_draw_rect(RenderItemID render_item_id, const Transform2D& transform,
	const Rect2D& dest_rect, Color color)
{
	RenderItem& item = _get_item(render_item_id);
	auto rect = item.alloc<RenderItem::CommandRect>();
	rect->type = RenderItem::CMD_RECT;
	rect->transform = transform;
	rect->rect = dest_rect;
	rect->color = color;
}

void RenderManager::render_item_draw_line(RenderItemID render_item_id, const Vector2& point1, const Vector2& point2, Color color)
{
	RenderItem& item = _get_item(render_item_id);
	auto line = item.alloc<RenderItem::CommandLine>();
	line->type = RenderItem::CMD_LINE;
	line->point1 = point1;
	line->point2 = point2;
	line->color = color;
}

void RenderManager::render_item_draw_circle(RenderItemID render_item_id, const Vector2& center, f32 radius, Color color)
{
	RenderItem& item = _get_item(render_item_id);
	auto circle = item.alloc<RenderItem::CommandCircle>();
	circle->type = RenderItem::CMD_CIRCLE;
	circle->center = center;
	circle->radius = radius;
	circle->color = color;
}

void RenderManager::render_item_draw_sprite(RenderItemID render_item_id, const Transform2D& transform,
	TextureID texture, const Rect2D& rect, const Rect2D& src_rect, Color mod_color, RenderFlags flags)
{
	RenderItem& item = _get_item(render_item_id);
	auto sprite = item.alloc<RenderItem::CommandSprite>();
	sprite->type = RenderItem::CMD_SPRITE;
	sprite->transform = transform;
	sprite->texture = texture;
	sprite->rect = rect;
	sprite->src_rect = src_rect;
	sprite->mod_color = mod_color;
	sprite->flags = flags;
}

void RenderManager::render_item_draw_ui_sprite(RenderItemID render_item_id, const Transform2D& transform, TextureID texture,
	const Rect2D& rect, const Rect2D& src_rect, Color mod_color, RenderFlags flags)
{
	RenderItem& item = _get_item(render_item_id);
	auto ui_sprite = item.alloc<RenderItem::CommandUISprite>();
	ui_sprite->type = RenderItem::CMD_UI_SPRITE;
	ui_sprite->transform = transform;
	ui_sprite->texture = texture;
	ui_sprite->rect = rect;
	ui_sprite->src_rect = src_rect;
	ui_sprite->mod_color = mod_color;
	ui_sprite->flags = flags;
}