#include "resource/font.h"

#include "graphics/graphics.h"
#include "resource/resource_manager.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SYSTEM_H
#include FT_MODULE_H

extern FT_MemoryRec_ ft_memory_rec;

void Font::init()
{
    Resource::init(RESOURCE_FONT);
    auto allocator = ResourceManager::get_allocator();
    data.glyphs = Array<Glyph>::with_allocator(allocator);
}


void Font::destroy()
{
	Resource::destroy();

    data.glyphs.destroy();
}


void Font::set_font_size(i32 new_size)
{
	data.font_size = new_size;
}

void Font::load_from_file(StringView file_path, i32 font_size)
{
    auto allocator = ResourceManager::get_allocator();
    path.set(file_path);

    Slice<u8> content = File::read_all(allocator, file_path);

    FT_Library library;
    FT_New_Library(&ft_memory_rec, &library);
    FT_Add_Default_Modules(library);

    FT_Face face;
    FT_New_Memory_Face(library, content.ptr(), content.len, 0, &face);
    FT_Set_Pixel_Sizes(face, font_size, font_size);

    u32 padding = 2;
    u32 row = 0;
    u32 column = padding;

    data.glyphs.resize(MinimumGlyphCount);
    for (FT_ULong glyph_index = 27; glyph_index < Font::MinimumGlyphCount; glyph_index++)
    {
        Glyph& glyph = data.glyphs[glyph_index];

        FT_Error error = FT_Load_Char(face, glyph_index, FT_LOAD_RENDER);
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

    data.font_size = font_size;

    FT_Done_Face(face);
    FT_Done_Library(library);

    allocator.free(content);
}
