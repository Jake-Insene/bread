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
constexpr StringView glsl_fragment_header = "#define FRAGMENT_SHADER\n";


enum CompileFlags
{
    CompileFlagNone = 0,
    CompileFlagHasVertex,
    CompileFlagHasFragment,
    CompileFlagCustomShader,
};

struct ShaderCode
{
    StringView main_code;
};

#define ADVANCE(c, text, count) \
    c = text[0]; \
    text = text.add(count);

static inline void _get_start_end(StringView source, StringView* start, StringView* end, StringView name)
{
    StringView current_text = source;
    *start = source;

    char c = 0;
    while (current_text.len != 0)
    {
        ADVANCE(c, current_text, 1);

        if (current_text.equals(name))
        {
            ADVANCE(c, current_text, name.len);

            start->len = (current_text.items - name.len) - start->items;
            end->items = current_text.items;
        }
    }

    end->len = current_text.items - (end->items);
}

static inline ShaderCode _parse_shader(StringView shader_source)
{
    // Keep it simple for now.
    if (shader_source.null())
        return ShaderCode();

    return ShaderCode
    {
        .main_code = shader_source
    };
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

static inline void _parse_program(const StringView& program, ShaderCode* vs, ShaderCode* fs)
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

    *vs = _parse_shader(vsstring);
    *fs = _parse_shader(fsstring);
}
#undef ADVANCE

static inline GLID _compile_shaders(StringView path, ShaderCode vscode, 
    ShaderCode fscode, StringView defines, u32 flags)
{
    auto all_defines = Scoped<String>(GLESMaterialManager::get_allocator());
    all_defines.add(defines);
    if (fscode.main_code.ptr())
    {
        all_defines.add("\n#define CUSTOM_FRAGMENT\n");
    }

    StringView vssource = GLESMaterialManager::data.vs_batch_shader;
    StringView fssource = GLESMaterialManager::data.fs_batch_shader;

    auto new_vs_formed_code = Scoped<String>(GLESMaterialManager::get_allocator());
    StringView vssource_start = {};
    StringView vssource_end = {};
    _get_start_end(vssource, &vssource_start, &vssource_end, "#VERTEXCODE");
    fmt::format<false>(new_vs_formed_code.writer(), "{}{}{}", vssource_start, vscode.main_code, vssource_end);

    StringView fssource_start = {};
    StringView fssource_end = {};
    auto new_fs_formed_code = Scoped<String>(GLESMaterialManager::get_allocator());
    _get_start_end(fssource, &fssource_start, &fssource_end, "#FRAGMENTCODE");
    fmt::format<false>(new_fs_formed_code.writer(), "{}{}{}", fssource_start, fscode.main_code, fssource_end);

    static constexpr i32 SourceCount = 5;
    const char* vs_sources[SourceCount] =
    {
        (const char*)glsl_version_header.ptr(),
        (const char*)glsl_vertex_header.ptr(),
        (const char*)GLESMaterialManager::data.glsl_shader_header.ptr(),
        (const char*)all_defines.view().ptr(),
        (const char*)new_vs_formed_code.view().ptr(),
    };

    GLint vs_lengths[SourceCount] =
    {
        (GLint)glsl_version_header.len,
        (GLint)glsl_vertex_header.len,
        (GLint)GLESMaterialManager::data.glsl_shader_header.len,
        (GLint)all_defines.count,
        (GLint)new_vs_formed_code.count,
    };

    const char* fs_sources[SourceCount] =
    {
        (const char*)glsl_version_header.ptr(),
        (const char*)glsl_fragment_header.ptr(),
        (const char*)GLESMaterialManager::data.glsl_shader_header.ptr(),
        (const char*)all_defines.view().ptr(),
        (const char*)new_fs_formed_code.view().ptr(),
    };

    GLint fs_lengths[SourceCount] =
    {
        (GLint)glsl_version_header.len,
        (GLint)glsl_fragment_header.len,
        (GLint)GLESMaterialManager::data.glsl_shader_header.len,
        (GLint)all_defines.count,
        (GLint)new_fs_formed_code.count,
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

        Fatal("Error compiling the vertex shader: '{}':\n{}", path, log_view);
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

        Fatal("Error compiling the fragment shader: '{}':\n{}", path, log_view);
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

        Fatal("Error linking the shader program: '{}':\n{}", path, log_view);
    }

    gl.glDeleteShader(vs);
    gl.glDeleteShader(fs);

    return program;
}

static inline GLID _compile_from_source(const StringView& path, const StringView& source, const StringView& defines, CompileFlags flags)
{
    ShaderCode vs = {};
    ShaderCode fs = {};
    if (flags & CompileFlagCustomShader)
    {
        _parse_program(source, &vs, &fs);
    }

    u32 new_flags = CompileFlagNone;
    if (vs.main_code.ptr())
    {
        new_flags |= CompileFlagHasVertex;
    }
    if (fs.main_code.ptr())
    {
        new_flags |= CompileFlagHasFragment;
    }

    GLID program = _compile_shaders(
        path, vs, fs, defines, new_flags
    );
    return program;
}

static inline GLID _compile_program(const StringView& program_path, const StringView& defines, CompileFlags flags)
{
    Slice<u8> program_content = File::read_all(GLESMaterialManager::get_allocator(), program_path);
    GLID program = _compile_from_source(
        program_path, mem::from_bytes<char>(program_content), 
        defines, flags
    );
    GLESMaterialManager::get_allocator().free(program_content);
    return program;
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
        data.sprite_material = create_material({});
        data.materials.get(data.sprite_material).program = _compile_program("shaders/bread/batch.gles.glsl", "#define SPRITE", CompileFlagNone);

        data.sprite_ui_material = create_material({});
        data.materials.get(data.sprite_ui_material).program = _compile_program("shaders/bread/batch.gles.glsl", "#define SPRITE\n#define SPRITE_UI", CompileFlagNone);

        data.quad_material = create_material({});
        data.materials.get(data.quad_material).program = _compile_program("shaders/bread/batch.gles.glsl", "#define QUAD", CompileFlagNone);

        data.lines_material = create_material({});
        data.materials.get(data.lines_material).program = _compile_program("shaders/bread/batch.gles.glsl", "#define PRIMITIVE", CompileFlagNone);

        data.circles_material = create_material({});
        data.materials.get(data.circles_material).program = _compile_program("shaders/bread/batch.gles.glsl", "#define CIRCLE", CompileFlagNone);
    }
}

void GLESMaterialManager::shutdown()
{
    destroy_material(data.sprite_material);
    destroy_material(data.sprite_ui_material);
    destroy_material(data.quad_material);
    destroy_material(data.lines_material);
    destroy_material(data.circles_material);

    get_allocator().free(data.glsl_shader_header);
    get_allocator().free(data.batch_shader);

	data.materials.destroy();
}

MaterialID GLESMaterialManager::create_material(const MaterialCreateInfo& create_info)
{
	MaterialID new_material = data.materials.add(GLESMaterial());
	GLESMaterial& material = data.materials.get(new_material);
	material.program = 0;

	return new_material;
}

void GLESMaterialManager::destroy_material(MaterialID material_id)
{
	GLESMaterial& material = data.materials.get(material_id);
	if (material.program != 0)
	{
		gl.glDeleteProgram(material.program);
		material.program = 0;
	}

	data.materials.remove(material_id);
}

void GLESMaterialManager::material_compile_from_file(MaterialID material_id, StringView path, StringView defines)
{
    GLESMaterial& material = data.materials.get(material_id);

    if (material.program != 0)
    {
        gl.glDeleteProgram(material.program);
        material.program = 0;
    }

    material.program = _compile_program(path, defines, CompileFlagCustomShader);
}

void GLESMaterialManager::material_compile_from_source(MaterialID material_id, StringView source, StringView defines)
{
    GLESMaterial& material = data.materials.get(material_id);

    if (material.program != 0)
    {
        gl.glDeleteProgram(material.program);
        material.program = 0;
    }

    material.program = _compile_from_source(
        "__source__", source, defines, CompileFlagCustomShader
    );
}

GLID GLESMaterialManager::material_get_program(MaterialID material_id)
{
    GLESMaterial& material = data.materials.get(material_id);
    DebugAssert(material.program != 0, "invalid program");

    return material.program;
}
