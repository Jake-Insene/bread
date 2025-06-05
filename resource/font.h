#pragma once
#include "collections/array.h"
#include "resource/resource.h"
#include "resource/resource_id.h"
#include "math/rect_2d.h"


struct Font : Resource
{
	RESOURCE(RESOURCE_FONT, .LoadFromAssets = true, .Extensions = ".ttf");

	static constexpr usize MinimumGlyphCount = 256;
	static constexpr usize DefaultFontSize = 256;

	struct Glyph
	{
		Rect2D src_rect;
	};

	struct InternalData
	{
		ResourceID font_texture;
		Array<Glyph> glyphs;
		i32 font_size;
	} data;

	void init();
	void destroy();

	void load_from_file(StringView file_path, i32 font_size);

	void set_font_size(i32 new_size);
	[[nodiscard]] i32 get_font_size() const { return data.font_size; }


};
