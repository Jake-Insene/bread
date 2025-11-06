#include "graphics/gles/gles_material_manager.h"

#include "collections/string.h"
#include "collections/scoped.h"
#include "debug/fail.h"
#include "fmt/fmt.h"
#include "graphics/gles/gles_vtable.h"

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

static inline GLID _compile_shader_for(const MaterialCompileInfo& cmp_info, StringView type_defines, 
    StringView formed_vs, StringView formed_fs)
{
    auto all_defines = Scoped<String>(GLESMaterialManager::get_allocator());
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

    static constexpr i32 SourceCount = 5;
    const char* vs_sources[SourceCount] =
    {
        (const char*)glsl_version_header.ptr(),
        (const char*)glsl_vertex_header.ptr(),
        (const char*)GLESMaterialManager::data.glsl_shader_header.ptr(),
        (const char*)all_defines.view().ptr(),
        (const char*)formed_vs.ptr(),
    };

    GLint vs_lengths[SourceCount] =
    {
        (GLint)glsl_version_header.len,
        (GLint)glsl_vertex_header.len,
        (GLint)GLESMaterialManager::data.glsl_shader_header.len,
        (GLint)all_defines.count,
        (GLint)formed_vs.len,
    };

    const char* fs_sources[SourceCount] =
    {
        (const char*)glsl_version_header.ptr(),
        (const char*)glsl_fragment_header.ptr(),
        (const char*)GLESMaterialManager::data.glsl_shader_header.ptr(),
        (const char*)all_defines.view().ptr(),
        (const char*)formed_fs.ptr(),
    };

    GLint fs_lengths[SourceCount] =
    {
        (GLint)glsl_version_header.len,
        (GLint)glsl_fragment_header.len,
        (GLint)GLESMaterialManager::data.glsl_shader_header.len,
        (GLint)all_defines.count,
        (GLint)formed_fs.len,
    };

    i32 status = GL_TRUE;
    char log[512] = {};
    
    GLID vs = gl.glCreateShader(GL_VERTEX_SHADER);
    gl.glShaderSource(vs, SourceCount, vs_sources, vs_lengths);
    gl.glCompileShader(vs);

    gl.glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        i32 len = 0;
        gl.glGetShaderInfoLog(vs, 512, &len, log);
        StringView log_view{ log, (usize)len };

        Fatal("Error compiling the vertex shader: '{}':\n{}", cmp_info.source_path, log_view);
    }

    GLID fs = gl.glCreateShader(GL_FRAGMENT_SHADER);
    gl.glShaderSource(fs, SourceCount, fs_sources, fs_lengths);
    gl.glCompileShader(fs);

    gl.glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        i32 len = 0;
        gl.glGetShaderInfoLog(fs, 512, &len, log);
        StringView log_view{ log, (usize)len };

        Fatal("Error compiling the fragment shader: '{}':\n{}", cmp_info.source_path, log_view);
    }

    GLID program = gl.glCreateProgram();
    gl.glAttachShader(program, vs);
    gl.glAttachShader(program, fs);
    gl.glLinkProgram(program);

    gl.glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        i32 len = 0;
        gl.glGetProgramInfoLog(program, 512, &len, log);
        StringView log_view{ log, (usize)len };

        Fatal("Error linking the shader program: '{}':\n{}", cmp_info.source_path, log_view);
    }

    gl.glDeleteShader(vs);
    gl.glDeleteShader(fs);

    return program;
}

static inline void _compile_shaders(GLESMaterialManager::GLESMaterial& material, const MaterialCompileInfo& cmp_info)
{
    constexpr StringView type_defines[] =
    {
        "#define SPRITE\n",
        "#define SPRITE\n#define SPRITE_UI\n",
        "#define QUAD\n",
        "#define PRIMITIVE\n",
        "#define CIRCLE\n",
    };

    StringView vssource = GLESMaterialManager::data.vs_batch_shader;
    StringView fssource = GLESMaterialManager::data.fs_batch_shader;

    auto new_vs_formed_code = Scoped<String>(GLESMaterialManager::get_allocator());
    StringView vssource_start = {};
    StringView vssource_end = {};
    _get_start_end(vssource, &vssource_start, &vssource_end, "#VERTEXCODE");
    fmt::format<false>(new_vs_formed_code.writer(), "{}{}{}", vssource_start, cmp_info.vscode, vssource_end);

    StringView fssource_start = {};
    StringView fssource_end = {};
    auto new_fs_formed_code = Scoped<String>(GLESMaterialManager::get_allocator());
    _get_start_end(fssource, &fssource_start, &fssource_end, "#FRAGMENTCODE");
    fmt::format<false>(new_fs_formed_code.writer(), "{}{}{}", fssource_start, cmp_info.fscode, fssource_end);

    GLID programs[] = {0, 0, 0, 0, 0};
    usize i = 0;
    for (auto def : type_defines)
    {
        GLID program = _compile_shader_for(cmp_info, def, new_vs_formed_code.view(), new_fs_formed_code.view());
        if (program == GLID(0))
            return;
        programs[i++] = program;
    }

    material.sprite_program = programs[0];
    material.sprite_ui_program = programs[1];
    material.quad_program = programs[2];
    material.lines_program = programs[3];
    material.circles_program = programs[4];
}

static inline void _destroy_shaders(GLESMaterialManager::GLESMaterial& material)
{
    if (material.sprite_program != 0)
    {
        gl.glDeleteProgram(material.sprite_program);
        material.sprite_program = 0;
    }

    if (material.sprite_ui_program != 0)
    {
        gl.glDeleteProgram(material.sprite_ui_program);
        material.sprite_ui_program = 0;
    }

    if (material.quad_program != 0)
    {
        gl.glDeleteProgram(material.quad_program);
        material.quad_program = 0;
    }

    if (material.lines_program != 0)
    {
        gl.glDeleteProgram(material.lines_program);
        material.lines_program = 0;
    }

    if (material.circles_program != 0)
    {
        gl.glDeleteProgram(material.circles_program);
        material.circles_program = 0;
    }
}

void GLESMaterialManager::initialize(const mem::Allocator& allocator)
{
	data.allocator = allocator;

    data.glsl_shader_header = File::read_all(allocator, "shaders/bread/header.gles.glsl");
    data.batch_shader = File::read_all(allocator, "shaders/bread/batch.gles.glsl");
    _parse_gles_shader(mem::from_bytes<char>(data.batch_shader), &data.vs_batch_shader, &data.fs_batch_shader);

	data.materials = FreeList<GLESMaterial, MaterialID>::with_size(allocator, 4);

	// Default materials
    {
        data.render_material = create_material(MaterialCreateInfo());
        MaterialCompileInfo cmp_info = {};
        cmp_info.source_path = "__default__";
        (void)material_compile_shader(data.render_material, cmp_info);
    }
}

void GLESMaterialManager::shutdown()
{
    destroy_material(data.render_material);

    get_allocator().free(data.glsl_shader_header);
    get_allocator().free(data.batch_shader);

	data.materials.destroy();
}

GLESMaterialManager::GLESMaterial& GLESMaterialManager::material_get(MaterialID material_id)
{
    GLESMaterial& material = data.materials.get(material_id);
    return material;
}

MaterialID GLESMaterialManager::create_material(const MaterialCreateInfo&)
{
	MaterialID new_material = data.materials.add(GLESMaterial());
    GLESMaterial& material = material_get(new_material);
    material.sprite_program = 0;
    material.sprite_ui_program = 0;
    material.quad_program = 0;
    material.lines_program = 0;
    material.circles_program = 0;

	return new_material;
}

void GLESMaterialManager::destroy_material(MaterialID material_id)
{
    GLESMaterial& material = material_get(material_id);
    _destroy_shaders(material);
	data.materials.remove(material_id);
}

Error GLESMaterialManager::material_compile_shader(MaterialID material_id, const MaterialCompileInfo& cmp_info)
{
    GLESMaterial& material = material_get(material_id);
    _destroy_shaders(material);
    _compile_shaders(material, cmp_info);

    return Ok;
}

