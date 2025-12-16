#include "render/material_manager.h"

#include "collections/string.h"
#include "collections/scoped.h"
#include "debug/fail.h"
#include "fmt/fmt.h"
#include "render/render_manager.h"


constexpr StringView glsl_version_header =
#if defined(BREAD_ANDROID)
    "#version 310 es\n";
#else
    "#version 450 core\n";
#endif

constexpr StringView glsl_vertex_header = "#define VERTEX_SHADER\n";
constexpr StringView glsl_fragment_header = "precision mediump float;\n#define FRAGMENT_SHADER\n";


enum CompileFlags
{
    CompileFlagNone = 0,
    CompileFlagHasVertex,
    CompileFlagHasFragment,
    CompileFlagCustomShader,
};

#define ADVANCE(c, text, count) \
    c = text[0]; \
    text = text.add(count);

#define ADVANCE_NO_C(text, count) \
    text = text.add(count);

static inline void _get_start_end(StringView source, StringView* start, StringView* end, StringView name)
{
    StringView current_text = source;
    *start = source;

    while (current_text.len != 0)
    {
        ADVANCE_NO_C(current_text, 1);

        if (current_text.equals(name))
        {
            ADVANCE_NO_C(current_text, name.len);

            start->len = (current_text.items - name.len) - start->items;
            end->items = current_text.items;
        }
    }

    end->len = current_text.items - (end->items);
}

static inline void _parse_gles_shader(const StringView& program, StringView* vsstring, StringView* fsstring)
{
    StringView* current_parsing = nullptr;
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
            if (!vsstring)
                continue;

            if (current_parsing)
            {
                current_parsing->len = (current_text.items - 7) - current_parsing->items;
            }
            vsstring->items = current_text.items;
            current_parsing = vsstring;
        }
        else if (current_text.equals("fragment"))
        {
            ADVANCE(c, current_text, 8);
            if (!fsstring)
                continue;

            if (current_parsing)
            {
                current_parsing->len = (current_text.items - 9) - current_parsing->items;
            }
            fsstring->items = current_text.items;
            current_parsing = fsstring;
        }
    }

    if (current_parsing)
    {
        current_parsing->len = current_text.items - current_parsing->items;
    }
}


#undef ADVANCE
#undef ADVANCE_NO_C

static inline Graphics::ProgramID _compile_shader_for(const MaterialManager::MaterialCompileInfo& cmp_info, StringView type_defines, 
    StringView formed_vs, StringView formed_fs)
{
    auto all_defines = Scoped<String>(MaterialManager::get_allocator());
    all_defines.add(cmp_info.defines);
    all_defines.add(type_defines);
    if (cmp_info.vscode.ptr())
    {
        all_defines.add("\n#define CUSTOM_VERTEX\n");
    }
    if (cmp_info.fscode.ptr())
    {
        all_defines.add("\n#define CUSTOM_FRAGMENT\n");
    }

    auto vs_source = Scoped<String>(MaterialManager::get_allocator());
    vs_source.add(glsl_version_header);
    vs_source.add(glsl_vertex_header);
    vs_source.add(StringView(mem::from_bytes<char>(MaterialManager::data.glsl_shader_header)));
    vs_source.add(all_defines.view());
    vs_source.add(formed_vs);

    auto fs_source = Scoped<String>(MaterialManager::get_allocator());
    fs_source.add(glsl_version_header);
    fs_source.add(glsl_fragment_header);
    fs_source.add(StringView(mem::from_bytes<char>(MaterialManager::data.glsl_shader_header)));
    fs_source.add(all_defines.view());
    fs_source.add(formed_fs);

    Graphics::ShaderInfo shaders[] =
    {
        {Graphics::SHADER_STAGE_VERTEX, cmp_info.source_path, mem::to_bytes(vs_source.view())},
        {Graphics::SHADER_STAGE_FRAGMENT, cmp_info.source_path, mem::to_bytes(fs_source.view())},
    };

    Graphics::ProgramCreateInfo pci =
    {
        .shaders = shaders,
    };
    Graphics::ProgramID program = Graphics::program_create(pci);
    
    return program;
}

static inline void _compile_shaders(MaterialManager::Material& material, const MaterialManager::MaterialCompileInfo& cmp_info)
{
    constexpr StringView type_defines[] =
    {
        "#define SPRITE\n",
        "#define SPRITE\n#define SPRITE_UI\n",
        "#define QUAD\n",
        "#define PRIMITIVE\n",
        "#define CIRCLE\n",
    };

    StringView vssource = MaterialManager::data.vs_batch_shader;
    StringView fssource = MaterialManager::data.fs_batch_shader;

    auto new_vs_formed_code = Scoped<String>(MaterialManager::get_allocator());
    StringView vssource_start = {};
    StringView vssource_end = {};
    _get_start_end(vssource, &vssource_start, &vssource_end, "#VERTEXCODE");
    fmt::format<false>(new_vs_formed_code.writer(), "{}{}{}", vssource_start, cmp_info.vscode, vssource_end);

    StringView fssource_start = {};
    StringView fssource_end = {};
    auto new_fs_formed_code = Scoped<String>(MaterialManager::get_allocator());
    _get_start_end(fssource, &fssource_start, &fssource_end, "#FRAGMENTCODE");
    fmt::format<false>(new_fs_formed_code.writer(), "{}{}{}", fssource_start, cmp_info.fscode, fssource_end);

    Graphics::ProgramID programs[] = 
    { 
        Graphics::ProgramID::invalid(),
        Graphics::ProgramID::invalid(),
        Graphics::ProgramID::invalid(),
        Graphics::ProgramID::invalid(),
        Graphics::ProgramID::invalid(),
    };
    usize i = 0;
    for (auto def : type_defines)
    {
        Graphics::ProgramID program = _compile_shader_for(cmp_info, def, new_vs_formed_code.view(), new_fs_formed_code.view());
        if (program == Graphics::ProgramID::invalid())
            return;
        programs[i++] = program;
    }

    material.sprite_program = programs[0];
    material.sprite_ui_program = programs[1];
    material.quads_program = programs[2];
    material.lines_program = programs[3];
    material.circles_program = programs[4];

	// Sprites Pipeline
    {
		Graphics::VertexBinding bindings[] =
		{
			{.binding = 0, .stride = sizeof(RenderManager::SpriteInstance), .input_rate = Graphics::INPUT_RATE_PER_INSTANCE }
		};

		Graphics::VertexAttribute attributes[] =
		{
			{0, 0, sizeof(Vector4) * 0, Graphics::VERTEX_FORMAT_RGBA32F},
			{1, 0, sizeof(Vector4) * 1, Graphics::VERTEX_FORMAT_RGBA32F},
			{2, 0, sizeof(Vector4) * 2, Graphics::VERTEX_FORMAT_RGBA32F},
			{3, 0, sizeof(Vector4) * 3, Graphics::VERTEX_FORMAT_RGBA32F},
			{4, 0, sizeof(Vector4) * 4, Graphics::VERTEX_FORMAT_RGBA32F},
		};

		Graphics::InputAssembly input_assembly =
		{
			.bindings = bindings,
			.attributes = attributes,
		};

		Graphics::PipelineCreateInfo sprite_pipeline_ci =
		{
			.usage = Graphics::PIPELINE_USAGE_GRAPHICS,
			.topology = Graphics::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.input_assembly = input_assembly,
			.pipeline_program = Graphics::ProgramID::invalid(),
		};

        sprite_pipeline_ci.pipeline_program = material.sprite_program;
	    material.sprite_pipeline = Graphics::pipeline_create(sprite_pipeline_ci);
	}

	// Sprites UI Pipeline
	{
		Graphics::VertexBinding bindings[] =
		{
			{.binding = 0, .stride = sizeof(RenderManager::SpriteUIInstance), .input_rate = Graphics::INPUT_RATE_PER_INSTANCE }
		};

		Graphics::VertexAttribute attributes[] =
		{
			{0, 0, sizeof(Vector4) * 0, Graphics::VERTEX_FORMAT_RGBA32F},
			{1, 0, sizeof(Vector4) * 1, Graphics::VERTEX_FORMAT_RGBA32F},
			{2, 0, sizeof(Vector4) * 2, Graphics::VERTEX_FORMAT_RGBA32F},
			{3, 0, sizeof(Vector4) * 3, Graphics::VERTEX_FORMAT_RGBA32F},
			{4, 0, sizeof(Vector4) * 4, Graphics::VERTEX_FORMAT_RGBA32F},
		};

		Graphics::InputAssembly input_assembly =
		{
			.bindings = bindings,
			.attributes = attributes,
		};

		Graphics::PipelineCreateInfo sprite_ui_pipeline_ci =
		{
			.usage = Graphics::PIPELINE_USAGE_GRAPHICS,
			.topology = Graphics::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.input_assembly = input_assembly,
			.pipeline_program = Graphics::ProgramID::invalid(),
		};

        sprite_ui_pipeline_ci.pipeline_program = material.sprite_ui_program;
	    material.sprite_ui_pipeline = Graphics::pipeline_create(sprite_ui_pipeline_ci);
	}

	// Quad Pipeline
	{
		Graphics::VertexBinding bindings[] =
		{
			{.binding = 0, .stride = sizeof(RenderManager::QuadInstance), .input_rate = Graphics::INPUT_RATE_PER_INSTANCE }
		};

		Graphics::VertexAttribute attributes[] =
		{
			{0, 0, sizeof(Vector4) * 0, Graphics::VERTEX_FORMAT_RGBA32F},
			{1, 0, sizeof(Vector4) * 1, Graphics::VERTEX_FORMAT_RGBA32F},
			{2, 0, sizeof(Vector4) * 2, Graphics::VERTEX_FORMAT_RGBA32F},
			{3, 0, sizeof(Vector4) * 3, Graphics::VERTEX_FORMAT_RGBA32F},
		};

		Graphics::InputAssembly input_assembly =
		{
			.bindings = bindings,
			.attributes = attributes,
		};

		Graphics::PipelineCreateInfo quads_pipeline_ci =
		{
			.usage = Graphics::PIPELINE_USAGE_GRAPHICS,
			.topology = Graphics::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.input_assembly = input_assembly,
			.pipeline_program = Graphics::ProgramID::invalid(),
		};

        quads_pipeline_ci.pipeline_program = material.quads_program;
	    material.quads_pipeline = Graphics::pipeline_create(quads_pipeline_ci);
	}

	// Lines Pipeline
	{
		Graphics::VertexBinding bindings[] =
		{
			{.binding = 0, .stride = sizeof(RenderManager::PrimitivePoint), .input_rate = Graphics::INPUT_RATE_PER_VERTEX }
		};

		Graphics::VertexAttribute attributes[] =
		{
			{0, 0, sizeof(Vector4) * 0, Graphics::VERTEX_FORMAT_RGBA32F},
		};

		Graphics::InputAssembly input_assembly =
		{
			.bindings = bindings,
			.attributes = attributes,
		};

		Graphics::PipelineCreateInfo lines_pipeline_ci =
		{
			.usage = Graphics::PIPELINE_USAGE_GRAPHICS,
			.topology = Graphics::PRIMITIVE_TOPOLOGY_LINE_LIST,
			.input_assembly = input_assembly,
			.pipeline_program = Graphics::ProgramID::invalid(),
		};

        lines_pipeline_ci.pipeline_program = material.lines_program;
	    material.lines_pipeline = Graphics::pipeline_create(lines_pipeline_ci);
	}

	// Circles Pipeline
	{
		Graphics::VertexBinding bindings[] =
		{
			{.binding = 0, .stride = sizeof(RenderManager::PrimitivePoint), .input_rate = Graphics::INPUT_RATE_PER_INSTANCE }
		};

		Graphics::VertexAttribute attributes[] =
		{
			{0, 0, sizeof(Vector4) * 0, Graphics::VERTEX_FORMAT_RGBA32F},
		};

		Graphics::InputAssembly input_assembly =
		{
			.bindings = bindings,
			.attributes = attributes,
		};

		Graphics::PipelineCreateInfo circles_pipeline_ci =
		{
			.usage = Graphics::PIPELINE_USAGE_GRAPHICS,
			.topology = Graphics::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.input_assembly = input_assembly,
			.pipeline_program = Graphics::ProgramID::invalid(),
		};

        circles_pipeline_ci.pipeline_program = material.circles_program;
	    material.circles_pipeline = Graphics::pipeline_create(circles_pipeline_ci);
	}
}

static inline void _destroy_shaders(MaterialManager::Material& material)
{
    if (material.sprite_program != Graphics::ProgramID::invalid())
    {
        Graphics::program_destroy(material.sprite_program);
        material.sprite_program = Graphics::ProgramID::invalid();
    }

    if (material.sprite_ui_program != Graphics::ProgramID::invalid())
    {
        Graphics::program_destroy(material.sprite_ui_program);
        material.sprite_ui_program = Graphics::ProgramID::invalid();
    }

    if (material.quads_program != Graphics::ProgramID::invalid())
    {
        Graphics::program_destroy(material.quads_program);
        material.quads_program = Graphics::ProgramID::invalid();
    }

    if (material.lines_program != Graphics::ProgramID::invalid())
    {
        Graphics::program_destroy(material.lines_program);
        material.lines_program = Graphics::ProgramID::invalid();
    }

    if (material.circles_program != Graphics::ProgramID::invalid())
    {
        Graphics::program_destroy(material.circles_program);
        material.circles_program = Graphics::ProgramID::invalid();
    }
}

void MaterialManager::initialize(const mem::Allocator& allocator)
{
	data.allocator = allocator;

    data.glsl_shader_header = File::read_all(allocator, "shaders/bread/header.gles.glsl");
    data.batch_shader = File::read_all(allocator, "shaders/bread/batch.gles.glsl");
    _parse_gles_shader(mem::from_bytes<char>(data.batch_shader), &data.vs_batch_shader, &data.fs_batch_shader);

	data.materials = FreeList<Material, MaterialID>::with_size(allocator, 4);

	// Default materials
    {
        data.render_material = material_create(MaterialCreateInfo());
        MaterialCompileInfo cmp_info = {};
        cmp_info.source_path = "__default__";
        (void)material_compile_shader(data.render_material, cmp_info);
    }
}

void MaterialManager::shutdown()
{
    material_destroy(data.render_material);

    get_allocator().free(data.glsl_shader_header);
    get_allocator().free(data.batch_shader);

	data.materials.destroy();
}

MaterialManager::Material& MaterialManager::material_get(MaterialID material_id)
{
    Material& material = data.materials.get(material_id);
    return material;
}

MaterialManager::MaterialID MaterialManager::material_create(const MaterialCreateInfo&)
{
	MaterialID new_material = data.materials.add(Material());
    Material& material = material_get(new_material);
    material.sprite_program = Graphics::ProgramID::invalid();
    material.sprite_ui_program = Graphics::ProgramID::invalid();
    material.quads_program = Graphics::ProgramID::invalid();
    material.lines_program = Graphics::ProgramID::invalid();
    material.circles_program = Graphics::ProgramID::invalid();

	return new_material;
}

void MaterialManager::material_destroy(MaterialID material_id)
{
    Material& material = material_get(material_id);
    _destroy_shaders(material);
	data.materials.remove(material_id);
}

Error MaterialManager::material_compile_shader(MaterialID material_id, const MaterialCompileInfo& cmp_info)
{
    Material& material = material_get(material_id);
    _destroy_shaders(material);
    _compile_shaders(material, cmp_info);

    return Ok;
}

