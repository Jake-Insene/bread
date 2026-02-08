#include "gui/canvas_element.h"

#include "resource/font.h"


void CanvasElement::init(const mem::Allocator& allocator)
{
    owner = nullptr;
    text = String::with_allocator(allocator);
    transform = Transform2D();
    element_rect = Rect2D(0, 0, 0, 0);
    element_color = Color(255, 255, 255, 255);
    visible_characters = MaxValue<usize>;

    data.font = nullptr;
    data.font_size = Font::DefaultFontSize;
}

void CanvasElement::deinit()
{
    text.destroy();
}

void CanvasElement::set_font(Font* new_font)
{
    if(data.font == new_font)
        return;

    data.font = new_font;
}

void CanvasElement::set_font_size(i32 new_font_size)
{
    if(data.font_size == new_font_size)
        return;

    data.font_size = new_font_size;
}


void CanvasElement::draw(RenderItemID render_item)
{
    if (!data.font || !text.count)
		return;

	Transform2D text_transform = transform;
    Rect2D text_rect = Rect2D();
    text_rect.position = text_transform.get_position();

	const Vector2 scale = transform.get_scale();
	const Vector2 begin_pos = transform.get_position();
	const Vector2 font_size = Vector2(f32(get_font_size()));

	const Font::FontTheme& font_theme = data.font->get_font_theme(get_font_size());
	
	usize characters_to_shown = visible_characters != MaxValue<usize> 
	? math::min(text.count, visible_characters) : text.count;
	
	f32 current_start_y = begin_pos.y;

    text_rect.size.height += get_font_size();
    
	for (usize i = 0; i < characters_to_shown; i++)
    {
        char character = text.get(i);
		const Font::Glyph& glyph = font_theme.glyphs.get(u8(character));
		const Vector2 advance = glyph.advance * scale;

        text_rect.size.width += advance.width;
    }

    // TODO: I need draw order to do this without overlapping the text.
    //RenderManager::render_item_draw_rect(
    //    render_item, 
    //    transform, 
    //    text_rect, 
    //    element_color
    //);

	for (usize i = 0; i < characters_to_shown; i++)
	{
		char character = text.get(i);
		const Font::Glyph& glyph = font_theme.glyphs.get(u8(character));
		const Vector2 advance = glyph.advance * scale;

		if (character == ' ' || character == '\0')
		{
			text_transform.translate(Vector2(advance.width, 0));
			continue;
		}
		else if (character == '\n')
		{
			current_start_y -= font_size.y;
			text_transform.set_position(Vector2(begin_pos.x, current_start_y));
			continue;
		}

		RenderManager::render_item_draw_sprite(
			render_item,
            text_transform, font_theme.font_atlas, 
			Rect2D(Vector2(), Vector2(glyph.src_rect.size)), glyph.src_rect,
			Color(255, 255, 255, 255),
			RenderManager::RENDER_FLAG_FONT_CHAR
		);

		text_transform.translate(
			Vector2(advance.width, 0)
		);
	}
}
