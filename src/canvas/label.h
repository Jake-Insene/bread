#pragma once
#include "canvas/canvas_object.h"
#include "resource/font.h"


struct Label : CanvasObject
{
	OBJECT(Label, CanvasObject)
    MARKS(MARK_RENDER)

	struct InternalData
	{
		i32 font_size = Font::DefaultFontSize;
		Font* font;

		Color color = Color(255, 255, 255, 255);
	} data;

	String text;
	usize visible_characters = MaxValue<usize>;

	void init(const CreateInfo&);
	void deinit();
	
	void render();

	void set_font(Font* new_font) { data.font = new_font; }
	[[nodiscard]] Font* get_font() { return data.font; }

	void set_font_size(i32 new_font_size);
	[[nodiscard]] i32 get_font_size() const { return data.font_size; }

	void set_color(Color new_color) { data.color = new_color; }
	[[nodiscard]] Color get_color() const { return data.color; }
};
