#pragma once
#include "collections/free_list.h"
#include "graphics/gles/gles_driver.h"
#include "resource/resource_id.h"


struct GLESMaterialManager
{
    struct GLESMaterial
    {
        GLID program;
    };

    struct InternalData
    {
        mem::Allocator allocator;

        usize allocated_bytes;

        FreeList<GLESMaterial, MaterialID> materials;
        MaterialID sprite_material;
        MaterialID sprite_ui_material;
        MaterialID quad_material;
        MaterialID lines_material;
        MaterialID circles_material;

        Slice<u8> glsl_shader_header;
        Slice<u8> batch_shader;
        StringView vs_batch_shader;
        StringView fs_batch_shader;
    };

    static inline InternalData data;

    [[nodiscard]] static mem::Allocator get_allocator() { return data.allocator; }

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

    static MaterialID get_sprite_material() { return data.sprite_material; }
    static MaterialID get_sprite_ui_material() { return data.sprite_ui_material; }
    static MaterialID get_quad_material() { return data.quad_material; }
    static MaterialID get_lines_material() { return data.lines_material; }
    static MaterialID get_circles_material() { return data.circles_material; }

    static MaterialID create_material(const MaterialCreateInfo& create_info);
    static void destroy_material(MaterialID material_id);

    static void material_compile_from_file(MaterialID material_id, StringView path, StringView defines);
    static void material_compile_from_source(MaterialID material_id, StringView source, StringView defines);

    static GLID material_get_program(MaterialID material_id);
};