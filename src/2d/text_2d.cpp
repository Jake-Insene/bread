#include "2d/text_2d.h"

#include "graphics/viewport.h"


void Text2D::init(const CreateInfo&)
{
	mark(MARK_RENDER);

	text = String::with_allocator(allocator);
}

void Text2D::deinit()
{
	text.destroy();
}

void Text2D::render()
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
		const Font::Glyph& glyph = font_theme.glyphs.get(character);
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

		TextureID id = glyph.char_texture;
		Vector2 extent = Vector2(Graphics::texture_get_size(id));
		draw_sprite(
			transform, id,
			Rect2D(Vector2(), extent), Rect2D(Vector2(), extent),
			color, Viewport::RENDER_FLAG_FLIP_V | Viewport::RENDER_FLAG_FONT_CHAR
		);

		transform.translate(
			Vector2(advance.x, 0)
		);
	}
}

void Text2D::set_font_size(i32 new_font_size)
{
	if (data.font_size == new_font_size)
		return;

	data.font_size = new_font_size;
}
