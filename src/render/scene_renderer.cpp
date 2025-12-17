#include "render/scene_renderer.h"

#include "engine/engine.h"
#include "graphics/graphics.h"
#include "math/projection.h"
#include "render/material_manager.h"
#include "render/viewport.h"
#include "render/render_manager.h"
#include "scene/scene_manager.h"


template<typename BatchInstance>
static inline void _update_batch_vb(SceneRenderer::Batch<BatchInstance>& batch, u32 stride)
{
	if (batch.instance_count == 0)
		return;

	Slice<u8> mem = Graphics::buffer_map_memory(batch.vb, 0, batch.instance_count * stride);
	mem::copy(mem, mem::to_bytes(batch.instances.slice(batch.instance_count)));
	Graphics::buffer_unmap_memory(batch.vb, mem);

	batch.instance_count = 0;
}

template<typename BatchInstance>
static inline void _update_sprite_batch_vb(SceneRenderer::SpriteBatch<BatchInstance>& batch, u32 stride)
{
	if (batch.instance_count == 0)
		return;

	Slice<u8> mem = Graphics::buffer_map_memory(batch.vb, 0, batch.instance_count * stride);
	mem::copy(mem, mem::to_bytes(batch.instances.slice(batch.instance_count)));
	Graphics::buffer_unmap_memory(batch.vb, mem);

	batch.instance_count = 0;
}

template<typename BatchInstance>
static inline void _end_prim_batch(Graphics::CommandBufferID cmd, SceneRenderer::Batch<BatchInstance>& batch, u32 stride)
{
	u32 offset = batch.buffer_offset;

	Graphics::command_buffer_bind_pipeline(cmd, batch.current_pipeline);
	Graphics::command_buffer_bind_index_buffer(cmd, SceneRenderer::data.global_ib, 0, Graphics::INDEX_TYPE_UINT8);
	Graphics::command_buffer_bind_vertex_buffers(
		cmd,
		0,
		Slice<Graphics::BufferID>(&batch.vb, 1),
		Slice<u32>(&offset, 1),
		Slice<u32>(&stride, 1)
	);

	if constexpr (IsSame<BatchInstance, SceneRenderer::PrimitivePoint>)
	{
		Graphics::command_buffer_draw(cmd, batch.current_instance_count, 1, 0, 0);
	}
	else
	{
		Graphics::command_buffer_draw_indexed(cmd, 6, batch.current_instance_count, 0, 0, 0);
	}

	batch.buffer_offset += stride * batch.current_instance_count;
	batch.current_instance_count = 0;
}

template<typename BatchInstance>
static inline void _end_sprite_batch(Graphics::CommandBufferID cmd, SceneRenderer::SpriteBatch<BatchInstance>& batch, u32 stride)
{
	u32 offset = batch.buffer_offset;

	Graphics::command_buffer_bind_pipeline(cmd, batch.current_pipeline);
	Graphics::command_buffer_bind_index_buffer(cmd, SceneRenderer::data.global_ib, 0, Graphics::INDEX_TYPE_UINT8);
	Graphics::command_buffer_bind_vertex_buffers(
		cmd,
		0,
		Slice<Graphics::BufferID>(&batch.vb, 1),
		Slice<u32>(&offset, 1),
		Slice<u32>(&stride, 1)
	);

	Graphics::command_buffer_set_texture_unit(
		cmd, 0, 0, Slice<Graphics::TextureID>(batch.texture_units, batch.texture_index)
	);

	Graphics::command_buffer_draw_indexed(cmd, 6, batch.current_instance_count, 0, 0, 0);

	batch.texture_index = 0;
	batch.buffer_offset += stride * batch.current_instance_count;
	batch.current_instance_count = 0;
}


// Global quad index buffer
static constexpr const u8 indices[] =
{
	0, 1, 2, 2, 3, 0
};

void SceneRenderer::initialize(const mem::Allocator& allocator)
{
    data.allocator = allocator;

	data.usable_texture_units = 16;

    // SwapChain
	Graphics::SwapChainCreateInfo scci =
	{
		.window = Engine::get_main_window().window_id,
		.present_mode = Graphics::PRESENT_MODE_IMMEDIATE,
		.format = Graphics::TEXTURE_FORMAT_RGBA8,
		.image_count = 2,
		.size = Engine::get_main_window().get_size(),
	};
	data.sc = Graphics::swap_chain_create(scci);
	data.render_target_size = Engine::get_main_window().get_size();

	data.render_targets[0] = Graphics::swap_chain_get_render_target(data.sc, 0);
	data.render_targets[1] = Graphics::swap_chain_get_render_target(data.sc, 1);

	data.present_index = 0;
	data.draw_index = 1;

	// Global Index Buffer
	Graphics::BufferCreateInfo global_ibci =
	{
		.usage = Graphics::BUFFER_USAGE_INDEX,
		.upload_rate = Graphics::UPLOAD_RATE_STATIC,
		.access = Graphics::MEMORY_ACCESS_READ_ONLY,
		.data = indices,
	};
	data.global_ib = Graphics::buffer_create(global_ibci);

	// Sprite batch
	{
		data.sprite_batch = {};

		Graphics::BufferCreateInfo bci =
		{
			.usage = Graphics::BUFFER_USAGE_VERTEX,
			.upload_rate = Graphics::UPLOAD_RATE_DYNAMIC,
			.access = Graphics::MEMORY_ACCESS_WRITE_ONLY,
			.data = Slice<u8>(nullptr, sizeof(SpriteInstance) * MaxInstancesPerBatch),
		};
		data.sprite_batch.vb = Graphics::buffer_create(bci);

		data.sprite_batch.instances = allocator.array<SpriteInstance>(MaxInstancesPerBatch);
	}

	// UI sprite batch
	{
		data.sprite_ui_batch = {};

		Graphics::BufferCreateInfo bci =
		{
			.usage = Graphics::BUFFER_USAGE_VERTEX,
			.upload_rate = Graphics::UPLOAD_RATE_DYNAMIC,
			.access = Graphics::MEMORY_ACCESS_WRITE_ONLY,
			.data = Slice<u8>(nullptr, sizeof(SpriteUIInstance) * MaxInstancesPerBatch),
		};
		data.sprite_ui_batch.vb = Graphics::buffer_create(bci);
		data.sprite_ui_batch.instances = allocator.array<SpriteUIInstance>(MaxInstancesPerBatch);
	}

	// Quad batch
	{
		data.quads_batch = {};

		Graphics::BufferCreateInfo bci =
		{
			.usage = Graphics::BUFFER_USAGE_VERTEX,
			.upload_rate = Graphics::UPLOAD_RATE_DYNAMIC,
			.access = Graphics::MEMORY_ACCESS_WRITE_ONLY,
			.data = Slice<u8>(nullptr, sizeof(QuadInstance) * MaxInstancesPerBatch),
		};
		data.quads_batch.vb = Graphics::buffer_create(bci);
		data.quads_batch.instances = allocator.array<QuadInstance>(MaxInstancesPerBatch);
	}

	// Lines batch
	{
		data.lines_batch = {};

		Graphics::BufferCreateInfo bci =
		{
			.usage = Graphics::BUFFER_USAGE_VERTEX,
			.upload_rate = Graphics::UPLOAD_RATE_DYNAMIC,
			.access = Graphics::MEMORY_ACCESS_WRITE_ONLY,
			.data = Slice<u8>(nullptr, sizeof(PrimitivePoint) * MaxPrimitivePointsPerBatch),
		};
		data.lines_batch.vb = Graphics::buffer_create(bci);
		data.lines_batch.instances = allocator.array<PrimitivePoint>(MaxPrimitivePointsPerBatch);
	}

	// Primitive circle batch
	{
		data.circles_batch = {};

		Graphics::BufferCreateInfo bci =
		{
			.usage = Graphics::BUFFER_USAGE_VERTEX,
			.upload_rate = Graphics::UPLOAD_RATE_DYNAMIC,
			.access = Graphics::MEMORY_ACCESS_WRITE_ONLY,
			.data = Slice<u8>(nullptr, sizeof(PrimitiveCircle) * MaxCirclesPerBatch),
		};
		data.circles_batch.vb = Graphics::buffer_create(bci);
		data.circles_batch.instances = allocator.array<PrimitiveCircle>(MaxCirclesPerBatch);
	}

	Graphics::BufferCreateInfo scene_ubci =
	{
		.usage = Graphics::BUFFER_USAGE_UNIFORM,
		.upload_rate = Graphics::UPLOAD_RATE_DYNAMIC,
		.access = Graphics::MEMORY_ACCESS_WRITE_ONLY,
		.data = Slice<const u8>(nullptr, sizeof(SceneUniform)),
	};
	data.scene_uniform = Graphics::buffer_create(scene_ubci);

	data.scene_data.viewport_transform = Mat4::identity();
	data.scene_data.scene_transform = Mat4::identity();
	data.scene_data_ubo_update = true;

	Graphics::CommandBufferCreateInfo cbci =
	{
		.usage = Graphics::COMMAND_BUFFER_USAGE_GRAPHICS,
	};
	data.command_buffer = Graphics::command_buffer_create(cbci);

	Graphics::QueueCreateInfo queueci =
	{
		.usage = Graphics::QUEUE_USAGE_GRAPHICS,
	};
	data.queue = Graphics::queue_create(queueci);
}

void SceneRenderer::shutdown()
{
    Graphics::swap_chain_destroy(data.sc);
	Graphics::queue_destroy(data.queue);
	Graphics::command_buffer_destroy(data.command_buffer);

	Graphics::buffer_destroy(data.global_ib);
	Graphics::buffer_destroy(data.scene_uniform);

	Graphics::buffer_destroy(data.sprite_batch.vb);
	get_allocator().free(mem::to_bytes(data.sprite_batch.instances));

	Graphics::buffer_destroy(data.sprite_ui_batch.vb);
	get_allocator().free(mem::to_bytes(data.sprite_ui_batch.instances));

	Graphics::buffer_destroy(data.quads_batch.vb);
	get_allocator().free(mem::to_bytes(data.quads_batch.instances));

	Graphics::buffer_destroy(data.lines_batch.vb);
	get_allocator().free(mem::to_bytes(data.lines_batch.instances));

	Graphics::buffer_destroy(data.circles_batch.vb);
	get_allocator().free(mem::to_bytes(data.circles_batch.instances));
}

void SceneRenderer::recreate_window()
{
    Graphics::swap_chain_destroy(data.sc);

	Graphics::SwapChainCreateInfo scci =
	{
		.window = Engine::get_main_window().window_id,
		.present_mode = Graphics::PRESENT_MODE_IMMEDIATE,
		.format = Graphics::TEXTURE_FORMAT_RGBA8,
		.image_count = 2,
		.size = Engine::get_configuration().viewport_size,
	};
	data.sc = Graphics::swap_chain_create(scci);

	data.render_targets[0] = Graphics::swap_chain_get_render_target(data.sc, 0);
	data.render_targets[1] = Graphics::swap_chain_get_render_target(data.sc, 1);

	data.present_index = 0;
	data.draw_index = 1;

	data.render_target_size = Engine::get_configuration().viewport_size;
}

void SceneRenderer::render_scene(Viewport* main_viewport)
{
    data.scene_data.time += SceneManager::get_delta_time();
	data.scene_data_ubo_update = true;
	//Graphics::RenderTargetID rt_id = main_viewport->rt;

	const Transform2D scene_transform = main_viewport->get_scene_transform();
	const Vector2 translation = scene_transform.get_position() * -1;
	const Vector2 scale = scene_transform.get_scale();

	data.scene_data.scene_transform =
		Mat4::translation(Vector3(translation.x, translation.y, 0))
		* Mat4::scaling(Vector3(scale.x, scale.y, 1))
		* Mat4::rotation_z(math::degrees(scene_transform.get_rotation()));

	data.scene_data.scene_transform.transpose();
	data.scene_data_ubo_update = true;

	if (main_viewport->must_sync)
	{
		_update_viewport_transform(main_viewport->viewport_size);
		main_viewport->must_sync = false;
	}
	_update_scene_uniform();

	Graphics::command_buffer_begin(data.command_buffer);
	Graphics::command_buffer_bind_render_target(data.command_buffer, main_viewport->rt);
	Graphics::command_buffer_set_uniform(data.command_buffer, 0, 0, Slice<Graphics::BufferID>(&data.scene_uniform, 1));
	Graphics::command_buffer_set_viewport(data.command_buffer, Rect2DI(Vector2I(), main_viewport->viewport_size));
	Graphics::command_buffer_clear(data.command_buffer, main_viewport->rt, Color(0, 0, 0, 255));

	for (RenderManager::RenderLayer& layer : RenderManager::data.order_table.layers)
	{
		for (RenderItemID& item_id : layer.items.iter())
		{
			RenderManager::RenderItem& item = RenderManager::_get_item(item_id);
			_render_item(item);
		}
	}

	// Updating buffer memory
	_update_sprite_batch_vb(data.sprite_batch, sizeof(SpriteInstance));
	_update_sprite_batch_vb(data.sprite_ui_batch, sizeof(SpriteUIInstance));
	_update_batch_vb(data.quads_batch, sizeof(QuadInstance));
	_update_batch_vb(data.lines_batch, sizeof(PrimitivePoint));
	_update_batch_vb(data.circles_batch, sizeof(PrimitiveCircle));

	if(main_viewport->material != nullptr)
	{
		Graphics::PipelineID screen_pipeline = MaterialManager::material_get(main_viewport->material->material_id).screen_pipeline;
		Graphics::command_buffer_bind_render_target(data.command_buffer, data.render_targets[data.draw_index]);
		Graphics::command_buffer_set_viewport(data.command_buffer, Rect2DI(Vector2I(), data.render_target_size));
		Graphics::command_buffer_clear(data.command_buffer, data.render_targets[data.draw_index], Color(0, 0, 0, 255));
		Graphics::command_buffer_bind_pipeline(data.command_buffer, screen_pipeline);

		Graphics::command_buffer_bind_index_buffer(data.command_buffer, data.global_ib, 0, Graphics::INDEX_TYPE_UINT8);

		Graphics::TextureID rt_texture = Graphics::render_target_get_texture(main_viewport->rt);
		Graphics::command_buffer_set_texture_unit(
			data.command_buffer, 0, 0, Slice<Graphics::TextureID>(&rt_texture, 1)
		);

		Graphics::command_buffer_draw_indexed(data.command_buffer, 6, 1, 0, 0, 0);
	}
	else
	{	
		Graphics::command_buffer_blit_framebuffer(
			data.command_buffer, main_viewport->rt, data.render_targets[data.present_index],
			Rect2DI(Vector2I(), main_viewport->viewport_size), Rect2DI(Vector2I(), data.render_target_size), 
			Graphics::TEXTURE_FILTER_NEAREST
		);
	}

	Graphics::command_buffer_end(data.command_buffer);
	Graphics::queue_execute_command_buffer(data.queue, Slice<Graphics::CommandBufferID>(&data.command_buffer, 1));

	data.sprite_batch.buffer_offset = 0;
	data.sprite_ui_batch.buffer_offset = 0;
	data.quads_batch.buffer_offset = 0;
	data.lines_batch.buffer_offset = 0;
	data.circles_batch.buffer_offset = 0;
}

void SceneRenderer::present_scene()
{
    Graphics::swap_chain_present(data.sc, data.present_index);
	data.present_index ^= 1;
	data.draw_index ^= 1;
}

void SceneRenderer::_bind_material()
{
	MaterialManager::MaterialID material_id = data.item_material != MaterialManager::MaterialID() ?
		data.item_material : MaterialManager::get_render_material();

	MaterialManager::Material& material = MaterialManager::material_get(material_id);
	// TODO: Get pipelines
	data.sprite_batch.current_pipeline = material.sprite_pipeline;
	data.sprite_ui_batch.current_pipeline = material.sprite_ui_pipeline;
	data.quads_batch.current_pipeline = material.quads_pipeline;
	data.lines_batch.current_pipeline = material.lines_pipeline;
	data.circles_batch.current_pipeline = material.circles_pipeline;
}

void SceneRenderer::_render_item(RenderManager::RenderItem& item)
{
	if (item.material != MaterialManager::MaterialID())
	{
		data.item_material = item.material;
	}
	_bind_material();

	RenderManager::RenderItem::Command* cmd = item.begin();
	for (; cmd != item.end(); cmd = item.get_command_at(cmd->next))
	{
		switch (cmd->type)
		{
		case RenderManager::RenderItem::CMD_RECT:
		{
			auto rect = reinterpret_cast<RenderManager::RenderItem::CommandRect*>(cmd);
			if (data.quads_batch.current_instance_count >= MaxInstancesPerBatch)
			{
				_end_prim_batch(data.command_buffer, data.quads_batch, sizeof(QuadInstance));
			}

			u32 index = data.quads_batch.instance_count;

			data.quads_batch.instances[index].transform_0 = rect->transform[0];
			data.quads_batch.instances[index].transform_1 = rect->transform[1];
			data.quads_batch.instances[index].transform_2 = rect->transform[2];

			data.quads_batch.instances[index].rect = rect->rect;
			data.quads_batch.instances[index].color = rect->color;

			data.quads_batch.instance_count++;
			data.quads_batch.current_instance_count++;
		}
		break;
		case RenderManager::RenderItem::CMD_LINE:
		{
			auto line = reinterpret_cast<RenderManager::RenderItem::CommandLine*>(cmd);
			if (data.lines_batch.current_instance_count >= MaxPrimitivePointsPerBatch)
			{
				_end_prim_batch(data.command_buffer, data.lines_batch, sizeof(PrimitivePoint));
			}

			u32 index = data.lines_batch.instance_count;

			data.lines_batch.instances[index].point = line->point1;
			data.lines_batch.instances[index].color = line->color;
			data.lines_batch.instances[index].flags = 0;

			data.lines_batch.instances[index + 1].point = line->point2;
			data.lines_batch.instances[index + 1].color = line->color;
			data.lines_batch.instances[index + 1].flags = 0;

			data.lines_batch.instance_count += 2;
			data.lines_batch.current_instance_count += 2;
		}
		break;
		case RenderManager::RenderItem::CMD_CIRCLE:
		{
			auto circle = reinterpret_cast<RenderManager::RenderItem::CommandCircle*>(cmd);
			if (data.circles_batch.current_instance_count >= MaxCirclesPerBatch)
			{
				_end_prim_batch(data.command_buffer, data.circles_batch, sizeof(PrimitiveCircle));
			}

			u32 index = data.circles_batch.instance_count;

			data.circles_batch.instances[index].point = circle->center;
			data.circles_batch.instances[index].color = circle->color;
			data.circles_batch.instances[index].radius = circle->radius;

			data.circles_batch.instance_count++;
			data.circles_batch.current_instance_count++;
		}
		break;
		case RenderManager::RenderItem::CMD_SPRITE:
		{
			auto sprite = reinterpret_cast<RenderManager::RenderItem::CommandSprite*>(cmd);

			if (data.sprite_batch.current_instance_count >= MaxInstancesPerBatch ||
				data.sprite_batch.texture_index >= data.usable_texture_units)
			{
				_end_sprite_batch(data.command_buffer, data.sprite_batch, sizeof(SpriteInstance));
			}

			Graphics::TextureID tex = sprite->texture;

			u32 tex_unit = u32(-1);
			for (u32 t = 0; t < data.sprite_batch.texture_index; t++)
			{
				if (data.sprite_batch.texture_units[t] == tex)
				{
					tex_unit = t;
					break;
				}
			}

			if (tex_unit == u32(-1))
			{
				tex_unit = data.sprite_batch.texture_index;
				data.sprite_batch.texture_units[data.sprite_batch.texture_index] = tex;
				data.sprite_batch.texture_index++;
			}

			u32 index = data.sprite_batch.instance_count;

			data.sprite_batch.instances[index].transform_0 = sprite->transform.get_column(0);
			data.sprite_batch.instances[index].transform_1 = sprite->transform.get_column(1);
			data.sprite_batch.instances[index].transform_2 = sprite->transform[2];

			data.sprite_batch.instances[index].unit = tex_unit;

			data.sprite_batch.instances[index].flags = 0;
			if (sprite->flags & RenderManager::RENDER_FLAG_FLIP_H)
			{
				data.sprite_batch.instances[index].flags |= FLAG_FLIP_H;
			}

			if (sprite->flags & RenderManager::RENDER_FLAG_FLIP_V)
			{
				data.sprite_batch.instances[index].flags |= FLAG_FLIP_V;
			}

			if (sprite->flags & RenderManager::RENDER_FLAG_FONT_CHAR)
			{
				data.sprite_batch.instances[index].flags |= FLAG_FONT_CHAR;
			}

			Vector2 texture_extent = Vector2(
				Graphics::texture_get_size(sprite->texture)
			);

			data.sprite_batch.instances[index].rect = sprite->rect;

			data.sprite_batch.instances[index].src_rect = Rect2D(
				sprite->src_rect.position / texture_extent,
				(sprite->src_rect.position + sprite->src_rect.size) / texture_extent
			);

			data.sprite_batch.instances[index].color = sprite->mod_color;

			data.sprite_batch.instance_count++;
			data.sprite_batch.current_instance_count++;
		}
		break;
		case RenderManager::RenderItem::CMD_UI_SPRITE:
		{
			auto ui_sprite = reinterpret_cast<RenderManager::RenderItem::CommandUISprite*>(cmd);

			if (data.sprite_ui_batch.current_instance_count >= MaxInstancesPerBatch ||
				data.sprite_ui_batch.texture_index >= data.usable_texture_units)
			{
				_end_sprite_batch(data.command_buffer, data.sprite_ui_batch, sizeof(SpriteUIInstance));
			}

			Graphics::TextureID tex = ui_sprite->texture;

			u32 tex_unit = u32(-1);
			for (u32 t = 0; t < data.sprite_ui_batch.texture_index; t++)
			{
				if (data.sprite_ui_batch.texture_units[t] == tex)
				{
					tex_unit = t;
					break;
				}
			}

			if (tex_unit == u32(-1))
			{
				tex_unit = data.sprite_ui_batch.texture_index;
				data.sprite_ui_batch.texture_units[data.sprite_ui_batch.texture_index] = tex;
				data.sprite_ui_batch.texture_index++;
			}

			u32 index = data.sprite_ui_batch.instance_count;

			data.sprite_ui_batch.instances[index].transform_0 = ui_sprite->transform.get_column(0);
			data.sprite_ui_batch.instances[index].transform_1 = ui_sprite->transform.get_column(1);
			data.sprite_ui_batch.instances[index].transform_2 = ui_sprite->transform[2];

			data.sprite_ui_batch.instances[index].unit = tex_unit;

			data.sprite_ui_batch.instances[index].flags = 0;
			if (ui_sprite->flags & RenderManager::RENDER_FLAG_FLIP_H)
			{
				data.sprite_ui_batch.instances[index].flags |= FLAG_FLIP_H;
			}

			if (ui_sprite->flags & RenderManager::RENDER_FLAG_FLIP_V)
			{
				data.sprite_ui_batch.instances[index].flags |= FLAG_FLIP_V;
			}

			if (ui_sprite->flags & RenderManager::RENDER_FLAG_FONT_CHAR)
			{
				data.sprite_ui_batch.instances[index].flags |= FLAG_FONT_CHAR;
			}

			Vector2 texture_extent = Vector2(
				Graphics::texture_get_size(ui_sprite->texture)
			);

			data.sprite_ui_batch.instances[index].rect = ui_sprite->rect;

			data.sprite_ui_batch.instances[index].src_rect = Rect2D(
				ui_sprite->src_rect.position / texture_extent,
				(ui_sprite->src_rect.position + ui_sprite->src_rect.size) / texture_extent
			);

			data.sprite_ui_batch.instances[index].color = ui_sprite->mod_color;

			data.sprite_ui_batch.instance_count++;
			data.sprite_ui_batch.current_instance_count++;
		}
		break;
		default:
			break;
		}
	}

	if (data.sprite_batch.current_instance_count > 0)
	{
		_end_sprite_batch(data.command_buffer, data.sprite_batch, sizeof(SpriteInstance));
	}

	if (data.sprite_ui_batch.current_instance_count > 0)
	{
		_end_sprite_batch(data.command_buffer, data.sprite_ui_batch, sizeof(SpriteUIInstance));
	}

	if (data.quads_batch.current_instance_count > 0)
	{
		_end_prim_batch(data.command_buffer, data.quads_batch, sizeof(QuadInstance));
	}

	if (data.lines_batch.current_instance_count > 0)
	{
		_end_prim_batch(data.command_buffer, data.lines_batch, sizeof(PrimitivePoint));
	}

	if (data.circles_batch.current_instance_count > 0)
	{
		_end_prim_batch(data.command_buffer, data.circles_batch, sizeof(PrimitiveCircle));
	}

	data.item_material = MaterialManager::MaterialID();
}

void SceneRenderer::_update_viewport_transform(const Vector2I& viewport_size)
{
	data.scene_data.viewport_transform = Projection::orthographic(
		0, f32(viewport_size.width), -f32(viewport_size.height), 0,
		1.f, -1.f
	);
	data.scene_data.viewport_transform.transpose();
	data.scene_data_ubo_update = true;
	_update_scene_uniform();
}

void SceneRenderer::_update_scene_uniform()
{
	if (!data.scene_data_ubo_update)
		return;

	data.scene_data_ubo_update = false;
	Slice<u8> uniform = Graphics::buffer_map_memory(
		data.scene_uniform, 0, sizeof(SceneUniform)
	);

	mem::copy(uniform, Slice<const u8>(reinterpret_cast<u8*>(&data.scene_data), sizeof(SceneUniform)));

	Graphics::buffer_unmap_memory(data.scene_uniform, uniform);
}
