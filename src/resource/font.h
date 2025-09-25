#pragma once
#include "collections/array.h"
#include "collections/error.h"
#include "graphics/graphics.h"
#include "resource/resource.h"
#include "resource/resource_id.h"
#include "math/rect_2d.h"


struct Texture2D;


struct Font : Resource
{
	RESOURCE(RESOURCE_FONT, LoadFromAssets, ResourceExtensions("ttf"));

	static constexpr usize MinimumGlyphCount = 256;
	static constexpr usize DefaultFontSize = 32;

	struct Glyph
	{
		TextureID char_texture;
		Rect2D src_rect;
		Vector2I advance;
	};

	struct FontTheme
	{
		Array<Glyph> glyphs;
		i32 font_size;
	};

	struct InternalData
	{
		Array<FontTheme> themes;
	} data;

	void init();
	void destroy();

	Error load(StringView file_path);

	[[nodiscard]] const FontTheme& get_font_theme(i32 font_size);

	const FontTheme& _theme_with_size(i32 font_size);
};
