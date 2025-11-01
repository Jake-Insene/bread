#pragma once
#include "collections/free_list.h"
#include "graphics/gles/gles_driver.h"
#include "resource/resource_id.h"


struct GLESMaterialManager
{
    struct GLESMaterial
    {
        GLID sprite_program;
        GLID sprite_ui_program;
        GLID quad_program;
        GLID lines_program;
        GLID circles_program;
    };

    struct InternalData
    {
        mem::Allocator allocator;

        usize allocated_bytes;

        FreeList<GLESMaterial, MaterialID> materials;
        MaterialID render_material;

        Slice<u8> glsl_shader_header;
        Slice<u8> batch_shader;
        StringView vs_batch_shader;
        StringView fs_batch_shader;
    };

    static inline InternalData data;

    [[nodiscard]] static mem::Allocator get_allocator() { return data.allocator; }

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();

    static GLESMaterial& material_get(MaterialID material_id);

    static MaterialID get_render_material() { return data.render_material; }

    static MaterialID create_material(const MaterialCreateInfo& create_info);
    static void destroy_material(MaterialID material_id);

    static Error material_compile_shader(MaterialID material_id, const MaterialCompileInfo& cmp_info);
};