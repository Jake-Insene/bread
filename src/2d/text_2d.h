#pragma once
#include "2d/object_2d.h"
#include "math/color.h"
#include "resource/font.h"


struct Text2D : Object2D
{
	OBJECT(Text2D, Object2D);

	struct InternalData
	{
		Font* font;
		i32 font_size = Font::DefaultFontSize;
	} data;

	Color color{ 255, 255, 255, 255 };
	String text;

	void init(const CreateInfo&);
	void deinit();

	void render();

	void set_font(Font* new_font) { data.font = new_font; }
	[[nodiscard]] Font* get_font() { return data.font; }

	void set_font_size(i32 new_font_size);
	[[nodiscard]] i32 get_font_size() const { return data.font_size; }
};
