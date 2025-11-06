#include "resource/material.h"

#include "graphics/graphics.h"
#include "io/file.h"
#include "resource/resource_manager.h"
#include "resource/resource_manager_internal.h"


#define ADVANCE(c, text, count) \
    c = text[0]; \
    text = text.add(count);
static inline void _parse_program(const StringView& program, StringView* vs, StringView* fs)
{
    StringView* current_parsing = nullptr;
    StringView vsstring = {};
    StringView fsstring = {};
    StringView current_text = program;

    char c = 0;
    while (current_text.len != 0)
    {
        ADVANCE(c, current_text, 1);
        if (c != '#')
            continue;

        if (current_text.equals("vertex"))
        {
            ADVANCE(c, current_text, 6);

            if (current_parsing)
            {
                current_parsing->len = (current_text.items - 7) - current_parsing->items;
            }
            vsstring.items = current_text.items;
            current_parsing = &vsstring;
        }
        else if (current_text.equals("fragment"))
        {
            ADVANCE(c, current_text, 8);

            if (current_parsing)
            {
                current_parsing->len = (current_text.items - 9) - current_parsing->items;
            }
            fsstring.items = current_text.items;
            current_parsing = &fsstring;
        }
    }

    if (current_parsing)
    {
        current_parsing->len = current_text.items - current_parsing->items;
    }

    *vs = vsstring;
    *fs = fsstring;
}
#undef ADVANCE


void Material::init()
{
	Resource::init(RESOURCE_MATERIAL);

	material_id = Graphics::create_material(
		MaterialCreateInfo{}
	);
}

void Material::destroy()
{
	Graphics::destroy_material(material_id);
}

Error Material::load_from_file(StringView file_path, StringView defines)
{
    if (File::exists(file_path) == false)
    {
        RMDebugInfo("Couldn't load the font '{}'", file_path);
        return MakeError(FileNotFound);
    }

    auto& allocator = ResourceManager::get_allocator();
    path.set(file_path);

	MaterialCompileInfo cmp_info = {};

	Slice<u8> bytes = File::read_all(allocator, file_path);
	cmp_info.source_path = file_path;
    _parse_program(mem::from_bytes<char>(bytes), &cmp_info.vscode, &cmp_info.fscode);
	cmp_info.defines = defines;


	Error result = Graphics::material_compile_shader(material_id, cmp_info);

	allocator.free(bytes);
	return result;
}


