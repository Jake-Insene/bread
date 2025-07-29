#pragma once
#include "canvas/canvas_object.h"
#include "resource/font.h"


struct Label : CanvasObject
{
	OBJECT(Label, CanvasObject);

	// As everything in a struct is public we need to hide data
	// that should not be modified/access directly, this also
	// resolve some namespace problems.
	struct InternalData
	{
		Texture2D* texture;
		i32 font_size = Font::DefaultFontSize;
	} data;

	Font* font;
	String text;

	void init(const CreateInfo&);
	void deinit();
	
	void render();

	void set_font_size(i32 new_font_size);
	[[nodiscard]] i32 get_font_size() const { return data.font_size; }
};
