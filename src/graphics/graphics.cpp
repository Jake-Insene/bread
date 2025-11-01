#include "graphics/graphics.h"

#include "graphics/gles/gles_driver.h"


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

void Graphics::render(Viewport* viewport)
{
	GraphicsDriver::render(viewport);
}

void Graphics::present(Viewport* viewport)
{
	GraphicsDriver::present(viewport);
}

TextureID Graphics::create_texture(const TextureCreateInfo& create_info)
{
	return GraphicsDriver::create_texture(create_info);
}

void Graphics::destroy_texture(TextureID tex_id)
{
	return GraphicsDriver::destroy_texture(tex_id);
}

RenderTargetID Graphics::create_render_target(const RenderTargetCreateInfo& create_info)
{
	return GraphicsDriver::create_render_target(create_info);
}

void Graphics::destroy_render_target(RenderTargetID rt_id)
{
	GraphicsDriver::destroy_render_target(rt_id);
}

MaterialID Graphics::create_material(const MaterialCreateInfo& create_info)
{
	return GraphicsDriver::create_material(create_info);
}

void Graphics::destroy_material(MaterialID material_id)
{
	GraphicsDriver::destroy_material(material_id);
}

RenderTargetID Graphics::get_main_render_target()
{
	return GraphicsDriver::get_main_render_target();
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

Error Graphics::material_compile_shader(MaterialID material_id, const MaterialCompileInfo& cmp_info)
{
	return GraphicsDriver::material_compile_shader(material_id, cmp_info);
}

