#include "graphics/gles/gles_utility.h"

#include "graphics/gles/gles_driver.h"
#include "graphics/gles/gles_vtable.h"
#include "graphics/gles/gles_shader.h"
#include "io/file.h"
#include "io/resource_manager.h"
#include "platform/platform_header.h"


namespace gles
{
    static void parse_program(StringView program, StringView* vsstring, StringView* fsstring)
    {
#define ADVANCE(count) \
    c = current_text[0]; \
    current_text = current_text.add(count); \
    
        StringView* current_parsing = nullptr;
        StringView current_text = program;
        char c = 0;
        while(current_text.len != 0)
        {
            ADVANCE(1);
            if(c == '#')
            {
                if(current_text.equals("vertex"))
                {
                    ADVANCE(6)
                    if(vsstring)
                    {
                        if(current_parsing)
                        {
                            current_parsing->len = (current_text.items - 7) - current_parsing->items;
                        }
                        vsstring->items = current_text.items;
                        current_parsing = vsstring;
                    }
                }
                else if(current_text.equals("fragment"))
                {
                    ADVANCE(8)
                    if(fsstring)
                    {
                        if(current_parsing)
                        {
                            current_parsing->len = (current_text.items - 9) - current_parsing->items;
                        }
                        fsstring->items = current_text.items;
                        current_parsing = fsstring;
                    }
                }
            }
        }
        
        if(current_parsing)
        {
            current_parsing->len = current_text.items - current_parsing->items;
        }
#undef ADVANCE
    }
    
    u32 compile_program(StringView program_path, StringView defines)
    {
        Slice<u8> program_content = File::read_all(GLESDriver::get_allocator(), program_path);
        
        StringView vsstring = {};
        StringView fsstring = {};
        parse_program(mem::from_bytes<char>(program_content), &vsstring, &fsstring);
        
        const char* sources[] =
        { 
#if defined(ENGINE_ANDROID)
            (const char*)glsl_es_shader_header.ptr(),
#else
			(const char*)glsl_core_shader_header.ptr(),
#endif
            (const char*)glsl_shader_constants,
            (const char*)defines.ptr(),
            nullptr,
        };
        
        GLint lengths[] =
        {
#if defined(ENGINE_ANDROID)
            (GLint)glsl_es_shader_header.len,
#else
            (GLint)glsl_core_shader_header.len,
#endif
            (GLint)glsl_shader_constants_len,
            (GLint)defines.len,
            0,
        };
        
        i32 source_count = sizeof(sources)/sizeof(const char*);
        
        i32 status = GL_TRUE;
        char log[512] = {};
        
        sources[source_count-1] = (const char*)vsstring.ptr();
        lengths[source_count-1] = (GLint)vsstring.len;
        
        u32 vs = gl.glCreateShader(GL_VERTEX_SHADER);
        gl.glShaderSource(vs, source_count, sources, lengths);
        gl.glCompileShader(vs);
        
        gl.glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
        if(status == GL_FALSE)
        {
            gl.glGetShaderInfoLog(vs, 512, nullptr, log);
            Fatal("Error compiling the vertex shader: '%s':\n%s", program_path, log);
        }
        
        sources[source_count-1] = (const char*)fsstring.ptr();
        lengths[source_count-1] = (GLint)fsstring.len;
        
        u32 fs = gl.glCreateShader(GL_FRAGMENT_SHADER);
        gl.glShaderSource(fs, source_count, sources, lengths);
        gl.glCompileShader(fs);
        
        gl.glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
        if(status == GL_FALSE)
        {
            gl.glGetShaderInfoLog(fs, 512, nullptr, log);
            Fatal("Error compiling the fragment shader: '%s':\n%s", program_path, log);
        }
        
        u32 program = gl.glCreateProgram();
        gl.glAttachShader(program, vs);
        gl.glAttachShader(program, fs);
        gl.glLinkProgram(program);
        
        gl.glGetProgramiv(program, GL_LINK_STATUS, &status);
        if(status == GL_FALSE)
        {
            gl.glGetProgramInfoLog(program, 512, nullptr, log);
            Fatal("Error linking the shader program: '%s'", program_path, log);
        }
        
        gl.glDeleteShader(vs);
        gl.glDeleteShader(fs);
        
        GLESDriver::get_allocator().free(program_content);
        
        return program;
    }
    
}
