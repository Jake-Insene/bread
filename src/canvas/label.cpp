#include "canvas/label.h"


void Label::init(const CreateInfo&)
{
	text = String::with_allocator(allocator);
}

void Label::deinit()
{
	text.destroy();
}

void Label::render()
{
	if (!data.font || !text.count)
		return;

	Transform2D transform = get_global_transform();
	const Vector2 scale = transform.get_scale();
	const Vector2 begin_pos = transform.get_position();
	const Vector2 font_size = Vector2(f32(get_font_size()));

	const Font::FontTheme& font_theme = data.font->get_font_theme(get_font_size());
	
	for (usize i = 0; i < text.count; i++)
	{
		char character = text.get(i);
		const Font::Glyph& glyph = font_theme.glyphs.get(u8(character));
		const Vector2 advance = Vector2(glyph.advance) * scale;

		if (character == ' ' || character == '\0')
		{
			transform.translate(Vector2(advance.x, 0));
			continue;
		}
		else if (character == '\n')
		{
			transform.set_position(Vector2(begin_pos.x, begin_pos.y - font_size.y));
			continue;
		}

		Graphics::TextureID texture_id = glyph.char_texture;
		Vector2 extent = Vector2(Graphics::texture_get_size(texture_id));
		
		draw_canvas_element(
			transform, texture_id, 
			Rect2D(Vector2(), extent), Rect2D(Vector2(), extent),
			get_color(), 
			RenderManager::RENDER_FLAG_FLIP_V 
			| RenderManager::RENDER_FLAG_FONT_CHAR
		);

		transform.translate(
			Vector2(advance.x, 0)
		);
	}
}

void Label::set_font_size(i32 new_font_size)
{
	if (data.font_size == new_font_size)
		return;

	data.font_size = new_font_size;
}
