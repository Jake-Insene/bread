#include "resource/font.h"

#include "graphics/graphics.h"
#include "resource/resource_manager.h"
#include "resource/resource_manager_internal.h"

#include <external/stb_truetype.h>


static inline bool _is_valid_glyph(u32 glyph_index)
{
    if (glyph_index == ' ' || glyph_index == '\0'
        || glyph_index == '\n')
        return false;

    return true;
}


static void _load_theme(stbtt_fontinfo* font, Font::FontTheme& theme)
{
    for (u32 glyph_index = 0; glyph_index < Font::MinimumGlyphCount; glyph_index++)
    {
        Font::Glyph& glyph = theme.glyphs.get(glyph_index);
        
        Opaque* bitmap = reinterpret_cast<Opaque*>(stbtt_GetCodepointBitmap(
            font, 0.f, stbtt_ScaleForPixelHeight(font, f32(theme.font_size)), 
            (int)glyph_index, &glyph.advance.width, &glyph.advance.height, 0, 0
        ));

        if (glyph_index == ' ')
        {
            f32 scale = stbtt_ScaleForPixelHeight(font, f32(theme.font_size));
            int advance;
            int lsb;
            stbtt_GetCodepointHMetrics(font, glyph_index, &advance, &lsb);

            glyph.advance.width = i32(advance * scale);

            stbtt_FreeBitmap(bitmap->cast<u8*>(), nullptr);
            continue;
        }

        auto pixels = Slice(bitmap->cast<u8*>(), glyph.advance.width * glyph.advance.height);
        if (_is_valid_glyph(glyph_index))
        {
            glyph.char_texture = Graphics::texture_create(
                Graphics::TextureCreateInfo
                {
                    .usage = Graphics::TEXTURE_USAGE_UPLOAD_ONCE,
                    .type = Graphics::TEXTURE_2D,
                    .format = Graphics::TEXTURE_FORMAT_R8,
                    .min_filter = Graphics::TEXTURE_FILTER_NEAREST,
                    .mag_filter = Graphics::TEXTURE_FILTER_NEAREST,
                    .size = Vector2I(glyph.advance.width, glyph.advance.height),
                    .pixels = pixels,
                }
            );
        }
        
        stbtt_FreeBitmap(bitmap->cast<u8*>(), nullptr);
    }
}


void Font::init()
{
    Resource::init(RESOURCE_FONT);
    auto& allocator = ResourceManager::get_allocator();
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

            Graphics::texture_destroy(glyph.char_texture);
        }

        theme.glyphs.destroy();
    }

    data.themes.destroy();
}


Error Font::load(StringView file_path)
{
    if (File::exists(file_path) == false)
    {
        RMDebugInfo("Couldn't load the font '{}'", file_path);
        return MakeError(FileNotFound);
    }

    auto& allocator = ResourceManager::get_allocator();
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
    auto& allocator = ResourceManager::get_allocator();
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
