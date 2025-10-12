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

	/*
	* Text modulate color.
	*/
	Color color{ 255, 255, 255, 255 };
	/*
	* The text requested to draw.
	*/
	String text;

	void init(const CreateInfo&);
	void deinit();

	void render();

	/*
	* Set the font used to draw.
	* 
	* @param new_font The new font to use.
	*/
	void set_font(Font* new_font) { data.font = new_font; }
	
	/*
	* @return The current font used to draw.
	*/
	[[nodiscard]] Font* get_font() { return data.font; }

	/*
	* Set the font size used to draw.
	* 
	* @param new_font_size The new font size to use.
	*/
	void set_font_size(i32 new_font_size);

	/*
	* @return The current font size used to draw.
	*/
	[[nodiscard]] i32 get_font_size() const { return data.font_size; }
};
