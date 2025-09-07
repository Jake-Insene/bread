#include "graphics/gles/gles_utility.h"

#include "graphics/gles/gles_driver.h"
#include "graphics/gles/gles_vtable.h"
#include "fs/file.h"
#include "platform/platform_header.h"
#include "resource/resource_manager.h"


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
        constexpr StringView glsl_version_header =
#if defined(ENGINE_ANDROID)
        "#version 310 es\n";
#else
        "#version 450 core\n";
#endif

        Slice<u8> program_content = File::read_all(GLESDriver::get_allocator(), program_path);
        
        StringView vs_string = {};
        StringView fs_string = {};
        parse_program(mem::from_bytes<char>(program_content), &vs_string, &fs_string);
        
        const char* sources[] =
        { 
			(const char*)glsl_version_header.ptr(),
            (const char*)defines.ptr(),
            nullptr,
        };
        
        GLint lengths[] =
        {
            (GLint)glsl_version_header.len,
            (GLint)defines.len,
            0,
        };
        
        i32 source_count = sizeof(sources)/sizeof(const char*);
        
        i32 status = GL_TRUE;
        char log[512] = {};
        
        sources[source_count-1] = (const char*)vs_string.ptr();
        lengths[source_count-1] = (GLint)vs_string.len;
        
        u32 vs = gl.glCreateShader(GL_VERTEX_SHADER);
        gl.glShaderSource(vs, source_count, sources, lengths);
        gl.glCompileShader(vs);
        
        gl.glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
        if(status == GL_FALSE)
        {
            i32 len = 0;
            gl.glGetShaderInfoLog(vs, 512, &len, log);
            StringView log_view{ log, (usize)len };

            Fatal("Error compiling the vertex shader: '{}':\n{}", program_path, log_view);
        }
        
        sources[source_count-1] = (const char*)fs_string.ptr();
        lengths[source_count-1] = (GLint)fs_string.len;

        u32 fs = gl.glCreateShader(GL_FRAGMENT_SHADER);
        gl.glShaderSource(fs, source_count, sources, lengths);
        gl.glCompileShader(fs);
        
        gl.glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
        if(status == GL_FALSE)
        {
            i32 len = 0;
            gl.glGetShaderInfoLog(fs, 512, &len, log);
            StringView log_view{ log, (usize)len };

            Fatal("Error compiling the fragment shader: '{}':\n{}", program_path, log_view);
        }
        
        u32 program = gl.glCreateProgram();
        gl.glAttachShader(program, vs);
        gl.glAttachShader(program, fs);
        gl.glLinkProgram(program);
        
        gl.glGetProgramiv(program, GL_LINK_STATUS, &status);
        if(status == GL_FALSE)
        {
            i32 len = 0;
            gl.glGetProgramInfoLog(program, 512, &len, log);
            StringView log_view{ log, (usize)len };

            Fatal("Error linking the shader program: '{}':\n{}", program_path, log_view);
        }
        
        gl.glDeleteShader(vs);
        gl.glDeleteShader(fs);
        
        GLESDriver::get_allocator().free(program_content);
        
        return program;
    }
    
}
