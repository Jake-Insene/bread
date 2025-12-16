#pragma once
#include "collections/error.h"
#include "collections/free_list.h"
#include "collections/string_view.h"
#include "graphics/graphics.h"
#include "resource/resource_id.h"


struct MaterialManager
{
    struct MaterialCreateInfo
    {
    };

    struct MaterialCompileInfo
    {
        StringView source_path;
        StringView vscode;
        StringView fscode;
        StringView defines;
    };

    using MaterialID = ID<u32, struct _MaterialTag>;

    struct Material
    {
        Graphics::PipelineID sprite_pipeline;
        Graphics::PipelineID sprite_ui_pipeline;
        Graphics::PipelineID quads_pipeline;
        Graphics::PipelineID lines_pipeline;
        Graphics::PipelineID circles_pipeline;

        Graphics::ProgramID sprite_program;
        Graphics::ProgramID sprite_ui_program;
        Graphics::ProgramID quads_program;
        Graphics::ProgramID lines_program;
        Graphics::ProgramID circles_program;
    };

    struct InternalData
    {
        mem::Allocator allocator;

        usize allocated_bytes;

        FreeList<Material, MaterialID> materials;
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

    static Material& material_get(MaterialID material_id);

    static MaterialID get_render_material() { return data.render_material; }

    static MaterialID material_create(const MaterialCreateInfo& create_info);
    static void material_destroy(MaterialID material_id);

    static Error material_compile_shader(MaterialID material_id, const MaterialCompileInfo& cmp_info);
};