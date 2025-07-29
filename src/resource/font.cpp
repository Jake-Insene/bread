#include "resource/font.h"

#include "graphics/graphics.h"
#include "resource/resource_manager.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SYSTEM_H
#include FT_MODULE_H

extern FT_MemoryRec_ ft_memory_rec;


static void _load_glyph(FT_Face face, Array<Font::Glyph>& glyphs)
{
    for (FT_ULong glyph_index = 27; glyph_index < Font::MinimumGlyphCount; glyph_index++)
    {
        Font::Glyph& glyph = glyphs[glyph_index];

        FT_Load_Char(face, glyph_index, FT_LOAD_RENDER);
        glyph.advance.x = face->glyph->advance.x >> 6;
        glyph.advance.y = face->glyph->advance.y >> 6;

        if (glyph_index == ' ') continue;

        glyph.char_texture = Graphics::texture_create(
            TextureCreateInfo
            {
                .type = TEXTURE_2D,
                .format = TEXTURE_FORMAT_R8,
                .min_filter = TEXTURE_FILTER_NEAREST,
                .mag_filter = TEXTURE_FILTER_NEAREST,
                .size = Vector2I(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                .pixels = Slice(face->glyph->bitmap.buffer, face->glyph->bitmap.width * face->glyph->bitmap.rows),
            }
        );
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
    
    for (auto& theme : data.themes)
    {
        theme.glyphs.destroy();
    }

    data.themes.destroy();
}


void Font::load_from_file(StringView file_path)
{
    if (File::exists(file_path) == false)
    {
        Fatal("Couldn't load the font '{}'", file_path);
        return;
    }

    auto allocator = ResourceManager::get_allocator();
    path.set(file_path);

    Slice<u8> content = File::read_all(allocator, file_path);

    FT_Library library;
    FT_New_Library(&ft_memory_rec, &library);
    FT_Add_Default_Modules(library);

    FT_Face face;
    FT_New_Memory_Face(library, content.ptr(), content.len, 0, &face);
    FT_Set_Pixel_Sizes(face, DefaultFontSize, DefaultFontSize);

    FontTheme& default_theme = data.themes.add(FontTheme());
    default_theme.glyphs = Array<Glyph>::with_allocator(allocator);
    default_theme.font_size = DefaultFontSize;

    default_theme.glyphs.resize(MinimumGlyphCount);
    _load_glyph(face, default_theme.glyphs);

    FT_Done_Face(face);
    FT_Done_Library(library);

    allocator.free(content);
}

const Font::FontTheme& Font::get_font_theme(i32 font_size)
{
    for (auto& theme : data.themes)
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

    FT_Library library;
    FT_New_Library(&ft_memory_rec, &library);
    FT_Add_Default_Modules(library);

    FT_Face face;
    FT_New_Memory_Face(library, content.ptr(), content.len, 0, &face);
    FT_Set_Pixel_Sizes(face, font_size, font_size);

    FontTheme& new_theme = data.themes.add(FontTheme());
    new_theme.glyphs = Array<Glyph>::with_allocator(allocator);
    new_theme.font_size = font_size;

    new_theme.glyphs.resize(MinimumGlyphCount);
    _load_glyph(face, new_theme.glyphs);

    FT_Done_Face(face);
    FT_Done_Library(library);

    allocator.free(content);

    return new_theme;
}
