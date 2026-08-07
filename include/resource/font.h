#pragma once
#include "collections/array.h"
#include "collections/error.h"
#include "graphics/gpu_resource_types.h"
#include "resource/resource.h"
#include "math/rect_2d.h"


struct Texture2D;


struct Font : Resource
{
	RESOURCE(RESOURCE_FONT, ResourceFlags::LoadFromAssets, ResourceExtensions("ttf"))

	static constexpr usize MinimumGlyphCount = 256;
	static constexpr usize DefaultFontSize = 32;

	struct Glyph
	{
		Rect2D src_rect;
		Vector2 advance;
		Vector2 size;
	};

	struct FontTheme
	{
		Array<Glyph> glyphs;
		i32 font_size;
		Vector2 atlas_size;
		Graphics::GPUTextureID font_atlas;

		FontTheme(Mem::Allocator* allocator);
		~FontTheme();
	};

	Array<FontTheme> themes;

	Font(const ResourceCreateInfo& info);
	virtual ~Font() override;

	Error load(StringView file_path);

	[[nodiscard]] const FontTheme& get_font_theme(i32 font_size);

	const FontTheme& _theme_with_size(i32 font_size);
};
