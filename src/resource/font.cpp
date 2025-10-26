#include "resource/font.h"

#include "graphics/graphics.h"
#include "resource/resource_manager.h"
#include "resource/resource_manager_internal.h"

#include <external/stb_truetype.h>


static void _load_theme(stbtt_fontinfo* font, Font::FontTheme& theme)
{
    for (u32 glyph_index = 27; glyph_index < Font::MinimumGlyphCount; glyph_index++)
    {
        Font::Glyph& glyph = theme.glyphs.get(glyph_index);
        if (glyph_index == ' ')
            continue;

        i32 width;
        i32 height;
        Opaque bitmap = stbtt_GetCodepointBitmap(
            font, 0.f, stbtt_ScaleForPixelHeight(font, f32(theme.font_size)), (int)glyph_index, &width, &height, 0, 0
        );

        auto pixels = Slice(bitmap.cast<u8*>(), width * height);

        glyph.char_texture = Graphics::create_texture(
            TextureCreateInfo
            {
                .type = TEXTURE_2D,
                .format = TEXTURE_FORMAT_R8,
                .min_filter = TEXTURE_FILTER_NEAREST,
                .mag_filter = TEXTURE_FILTER_NEAREST,
                .size = Vector2I(width, height),
                .pixels = pixels,
            }
        );
        
        glyph.advance.x = width;
        glyph.advance.y = height;
        stbtt_FreeBitmap(bitmap.cast<u8*>(), nullptr);
    }
}


void Font::init()
{
    Resource::init(RESOURCE_FONT);
    auto allocator = ResourceManager::get_allocator();
    data.themes = Array<FontTheme>::with_allocator(allocator);
}


void Font::destroy()
{
	Resource::destroy();
    
    for (auto& theme : data.themes.iter())
    {
        for (auto& glyph : theme.glyphs.iter())
        {
            if (glyph.char_texture == ResourceID())
                continue;

            Graphics::destroy_texture(glyph.char_texture);
        }

        theme.glyphs.destroy();
    }

    data.themes.destroy();
}


Error Font::load(StringView file_path)
{
    if (File::exists(file_path) == false)
    {
        RMFatal("Couldn't load the font '{}'", file_path);
        return MakeError(FileNotFound);
    }

    auto allocator = ResourceManager::get_allocator();
    path.set(file_path);

    Slice<u8> content = File::read_all(allocator, file_path);

    stbtt_fontinfo font;
    stbtt_InitFont(&font, content.ptr(), stbtt_GetFontOffsetForIndex(content.ptr(), 0));

    FontTheme& default_theme = data.themes.add(FontTheme());
    default_theme.glyphs = Array<Glyph>::with_allocator(allocator);
    default_theme.font_size = DefaultFontSize;

    default_theme.glyphs.resize(MinimumGlyphCount);
    _load_theme(&font, default_theme);
    
    allocator.free(content);

    return Ok;
}

const Font::FontTheme& Font::get_font_theme(i32 font_size)
{
    for (auto& theme : data.themes.iter())
    {
        if (theme.font_size == font_size)
            return theme;
    }

    return _theme_with_size(font_size);
}

const Font::FontTheme& Font::_theme_with_size(i32 font_size)
{
    auto allocator = ResourceManager::get_allocator();
    Slice<u8> content = File::read_all(allocator, path.view());

    stbtt_fontinfo font;
    stbtt_InitFont(&font, content.ptr(), stbtt_GetFontOffsetForIndex(content.ptr(), 0));

    FontTheme& new_theme = data.themes.add(FontTheme());
    new_theme.glyphs = Array<Glyph>::with_allocator(allocator);
    new_theme.font_size = font_size;

    new_theme.glyphs.resize(MinimumGlyphCount);
    _load_theme(&font, new_theme);

    allocator.free(content);

    return new_theme;
}
