#include "graphics/graphics.h"

#include "graphics/gles/gles_driver.h"

#if ENABLE_GRAPHICS_DRIVERS
void Graphics::initialize(const mem::Allocator& allocator, DriverType driver)
{
	switch (driver)
	{
	case Graphics::GLES:
		vtable = GLESDriver::get_vtable();
		break;
	default:
		FailOn(true, "Invalid graphics driver");
		break;
	}

	vtable.initialize(allocator);
}
#else

using GraphicsDriver = GLESDriver;

void Graphics::initialize(const mem::Allocator& allocator)
{
	GraphicsDriver::initialize(allocator);
}

void Graphics::shutdown()
{
	GraphicsDriver::shutdown();
}

void Graphics::recreate()
{
	GraphicsDriver::recreate();
}

void Graphics::destroy()
{
	GraphicsDriver::destroy();
}

void Graphics::render(RenderTargetID rt_id, const RenderInfo& ri)
{
	GraphicsDriver::render(rt_id, ri);
}

void Graphics::present(RenderTargetID rt_id)
{
	GraphicsDriver::present(rt_id);
}

void Graphics::add_cmd(const RenderCommand& cmd)
{
	GraphicsDriver::add_cmd(cmd);
}

Graphics::TextureID Graphics::create_texture(const TextureCreateInfo& create_info)
{
	return GraphicsDriver::create_texture(create_info);
}

void Graphics::destroy_texture(TextureID tex_id)
{
	return GraphicsDriver::destroy_texture(tex_id);
}

Graphics::RenderTargetID Graphics::create_render_target(const RenderTargetCreateInfo& create_info)
{
	return GraphicsDriver::create_render_target(create_info);
}

void Graphics::destroy_render_target(RenderTargetID rt_id)
{
	GraphicsDriver::destroy_render_target(rt_id);
}

void Graphics::texture_set_image(TextureID tex_id, Image* img)
{
	GraphicsDriver::texture_set_image(tex_id, img);
}

Vector2I Graphics::texture_get_size(TextureID tex_id)
{
	return GraphicsDriver::texture_get_size(tex_id);
}

void Graphics::render_target_set_size(RenderTargetID rt_id, const Vector2I& new_size)
{
	GraphicsDriver::render_target_set_size(rt_id, new_size);
}

Vector2I Graphics::render_target_get_size(RenderTargetID rt_id)
{
	return GraphicsDriver::render_target_get_size(rt_id);
}

#endif


// Utilities

void Graphics::draw_texture(const Transform2D& transform, const Vector2& dest_extent, const Rect2D& src_rect,
	TextureID texture_id, Color mod_color, RenderCommand::BatchFlags flags)
{
	add_cmd(
		RenderCommand
		{
			.type = RenderCommand::DRAW_SPRITE,
			.sprite =
			{
				.transform = transform,
				.dest_extent = dest_extent,
				.src_rect = src_rect,
				.texture = texture_id,
				.color = mod_color,
				.flags = flags,
			},
		}
	);
}

void Graphics::draw_canvas_element(const Transform2D& transform, const Vector2& dest_extent, const Rect2D& src_rect,
	TextureID texture_id, Color mod_color, RenderCommand::BatchFlags flags)
{
	add_cmd(
		RenderCommand
		{
			.type = RenderCommand::DRAW_CANVAS_ELEMENT,
			.sprite =
			{
				.transform = transform,
				.dest_extent = dest_extent,
				.src_rect = src_rect,
				.texture = texture_id,
				.color = mod_color,
				.flags = flags,
			},
		}
	);
}

void Graphics::draw_quad(const Transform2D& transform, const Vector2& size, const Color& color)
{
	Graphics::add_cmd(
		RenderCommand
		{
			.type = RenderCommand::DRAW_QUAD,
			.quad =
			{
				.transform = transform,
				.size = size,
				.color = color,
			}
		}
	);
}

void Graphics::draw_line(const Vector2& start, const Vector2& end, Color color)
{
	Graphics::add_cmd(
		RenderCommand
		{
			.type = RenderCommand::DRAW_LINE,
			.line =
			{
				.start = start,
				.end = end,
				.color = color,
			}
		}
	);
}

void Graphics::draw_circle(const Vector2& point, f32 radius, Color color)
{
	Graphics::add_cmd(
		RenderCommand
		{
			.type = RenderCommand::DRAW_CIRCLE,
			.circle =
			{
				.point = point,
				.color = color,
				.radius = radius,
			}
		}
	);
}

