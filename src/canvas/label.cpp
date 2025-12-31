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
	
	usize characters_to_shown = visible_characters != MaxValue<usize> 
	? math::min(text.count, visible_characters) : text.count;
	
	f32 current_start_y = begin_pos.y;

	for (usize i = 0; i < characters_to_shown; i++)
	{
		char character = text.get(i);
		const Font::Glyph& glyph = font_theme.glyphs.get(u8(character));
		const Vector2 advance = glyph.advance * scale;

		if (character == ' ' || character == '\0')
		{
			transform.translate(Vector2(advance.width, 0));
			continue;
		}
		else if (character == '\n')
		{
			current_start_y -= font_size.y;
			transform.set_position(Vector2(begin_pos.x, current_start_y));
			continue;
		}

		draw_canvas_element(
			transform, font_theme.font_atlas, 
			Rect2D(Vector2(), Vector2(glyph.src_rect.size)), glyph.src_rect,
			get_color(),
			RenderManager::RENDER_FLAG_FONT_CHAR
		);

		transform.translate(
			Vector2(advance.width, 0)
		);
	}
}

void Label::set_font_size(i32 new_font_size)
{
	if (data.font_size == new_font_size)
		return;

	data.font_size = new_font_size;
}
