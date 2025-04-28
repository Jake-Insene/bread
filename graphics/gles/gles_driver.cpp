#include "graphics/gles/gles_driver.h"

#include "debug/debug.h"
#include "graphics/egl/egl.h"
#include "graphics/gles/gles_vtable.h"
#include "graphics/gles/gles_utility.h"
#include "graphics/gles/gles_cmd_proc.h"
#include "graphics/gles/gles_shader.h"
#include "io/texture.h"
#include "io/resource_manager.h"

#include <cstdio>

static inline void debug_callback(
    GLenum, GLenum, GLuint, GLenum severity,
    GLsizei length, const GLchar* message,
    const void*
)
{
    switch(severity)
    {
    case GL_DEBUG_SEVERITY_LOW:
        Log::info("GLInfo: %.*s\n", length, message);
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        Log::warning("GLWarn: %.*s\n", length, message);
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        Log::error("GLError: %.*s\n", length, message);
        break;
    default:
        break;
    }
}

static inline i32 get_texture_target(TextureType type)
{
    switch(type)
    {
    case TEXTURE_2D:
        return GL_TEXTURE_2D;
    default:
        DebugAssert(false, "invalid texture type");
        break;
    }
    
    return 0;
}

static inline i32 get_texture_format(TextureFormat format)
{
    switch(format)
    {
    case TEXTURE_FORMAT_RGB8:
        return GL_RGB;
    case TEXTURE_FORMAT_RGBA8:
        return GL_RGBA;    
    default:
        DebugAssert(false, "invalid texture format");
        break;
    }
    
    return 0;
}

static inline i32 get_texture_filter(TextureFilter filter)
{
    switch(filter)
    {
    case TEXTURE_FILTER_NEAREST:
        return GL_NEAREST;
    case TEXTURE_FILTER_LINEAR:
        return GL_LINEAR;    
    default:
        DebugAssert(false, "invalid texture filter");
        break;
    }
    
    return 0;
}


Graphics::VTable GLESDriver::get_vtable()
{
    return Graphics::VTable
    {
        
        .initialize = &GLESDriver::initialize,
        .shutdown = &GLESDriver::shutdown,
		.recreate = &GLESDriver::recreate,
		.destroy = &GLESDriver::destroy,
        
        .render = &GLESDriver::render,
        .present = &GLESDriver::present,
        
        .add_cmd = &GLESDriver::add_cmd,
        
        .texture_create = &GLESDriver::texture_create,
        .texture_set_image = &GLESDriver::texture_set_image,
        .texture_get_size = &GLESDriver::texture_get_size,
        
        .render_target_create = &GLESDriver::render_target_create,
        .render_target_get_size = &GLESDriver::render_target_get_size,
    };
}

void GLESDriver::initialize(mem::Allocator& allocator)
{
    Log::info("Initializing renderer...");

    data.allocator = allocator;
    data.textures = Array<GLESTexture>::with_allocator(
        data.allocator
    );
    data.render_targets = Array<GLESRenderTarget>::with_allocator(
        data.allocator
    );
    
    EGL::initialize(allocator);
    
    gl.glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, (i32*)&data.limits.max_texture_units);

    glsl_shader_constants_len = std::snprintf(
            glsl_shader_constants,
            gles_shader_constants_buffer_size,
            R"(
            #define MAX_TEXTURE_UNITS %d
        )",
            (i32)data.limits.max_texture_units
    );

    init_context();
}

void GLESDriver::shutdown()
{
    Debug::info("Shutting down renderer...");
    
    destroy_context();
    data.textures.destroy();
    data.render_targets.destroy();
    
    EGL::shutdown();
}

void GLESDriver::recreate()
{
    EGL::recreate_window_surface();

    Vector2I size = EGL::get_surface_size();
    gl.glViewport(0, 0, size.x, size.y);
}

void GLESDriver::destroy()
{
    EGL::uncreate_window_surface();
}

void GLESDriver::init_context()
{
    // Check openGL on the system
    GLint opengl_info[] = {GL_VENDOR, GL_RENDERER, GL_VERSION};
    for (auto name : opengl_info)
    {
        auto info = gl.glGetString(name);
        Debug::info("OpenGL Info: %s", info);
    }
    
    GLint num_extensions = 0;
    gl.glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    Debug::info("OpenGL Extensions: %d", num_extensions);
    for(GLint i = 0; i < num_extensions; i++)
    {
        Debug::info("%s", gl.glGetStringi(GL_EXTENSIONS, i));
    }
    
    // Debugging
#if DEBUG
    gl.glEnable(GL_DEBUG_OUTPUT);
    gl.glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // Immediate debug messages
    gl.glDebugMessageCallback(debug_callback, nullptr);
    gl.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#endif
    
    Debug::info("Texture Units: %llu", data.limits.max_texture_units);
    
    Vector2I size = EGL::get_surface_size();
    Debug::info("Viewport: W=%i H=%i", size.x, size.y);
    gl.glViewport(0, 0, size.x, size.y);

    GLESCommandProcessor::initialize(data.allocator);
}

void GLESDriver::destroy_context()
{
    GLESCommandProcessor::shutdown();
    
    // Unload graphics resources.
    for(auto& texture : data.textures)
    {
        gl.glDeleteTextures(1, &texture.gl_id);
    }
    
    for(auto& rt : data.render_targets)
    {
        gl.glDeleteTextures(1, &rt.color_buffer);
        gl.glDeleteFramebuffers(1, &rt.gl_id);
    }
}

void GLESDriver::render()
{
    GLESCommandProcessor::render();
}

void GLESDriver::present()
{
    Vector2I size = EGL::get_surface_size();
    auto& rt = render_targets_get(GLESCommandProcessor::get_current_fb());
    
    if (GLESCommandProcessor::data.state.current_fbo != 0)
    {
        gl.glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    gl.glViewport(0, 0, size.width, size.height);
    
    gl.glBindFramebuffer(GL_READ_FRAMEBUFFER, rt.gl_id);

    gl.glBlitFramebuffer(
        0, 0, rt.size.width, rt.size.height,
        0, 0, size.width, size.height,
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST
    );

    if(GLESCommandProcessor::data.state.current_fbo != 0)
    {
        gl.glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        GLESCommandProcessor::data.state.last_fbo = GLESCommandProcessor::get_current_fbo();
        GLESCommandProcessor::set_current_fbo(0);
    }
    
    EGL::present();
}

void GLESDriver::add_cmd(const RenderCommand& cmd)
{
    (void)GLESCommandProcessor::data.commands.add(cmd);
}

ResourceID GLESDriver::texture_create(const TextureCreateInfo& create_info)
{
    GLESTexture& tex = textures_allocate();
    tex.image = nullptr;
    tex.size = create_info.size;
    
    tex.gl_internal_format = GL_RGBA8;
    tex.gl_target = get_texture_target(create_info.type);
    tex.gl_format = get_texture_format(create_info.format);
    tex.gl_min_filter = get_texture_filter(create_info.min_filter);
    tex.gl_mag_filter = get_texture_filter(create_info.mag_filter);
    
    tex.gl_id = make_texture_and_fill(
        tex.gl_min_filter, tex.gl_mag_filter, GL_CLAMP_TO_EDGE,
        tex.size, tex.gl_internal_format, create_info.pixels.items,
        tex.gl_format, tex.gl_target
    );
    
    return tex.self_id;
}

Vector2I GLESDriver::texture_get_size(ResourceID tid)
{
    return textures_get(tid).size;
}

void GLESDriver::texture_set_image(ResourceID tid, Image* image)
{
    textures_get(tid).image = image;
}

ResourceID GLESDriver::render_target_create(const RenderTargetCreateInfo& create_info)
{
    GLESRenderTarget& rt = render_targets_allocate();
    rt.size = create_info.size;
    
    gl.glGenFramebuffers(1, &rt.gl_id);
    gl.glBindFramebuffer(GL_FRAMEBUFFER, rt.gl_id);

    gl.glGenTextures(1, &rt.color_buffer);
    gl.glBindTexture(GL_TEXTURE_2D, rt.color_buffer);

    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl.glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA8,
        create_info.size.width, create_info.size.height,
        0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr
    );

    gl.glBindTexture(GL_TEXTURE_2D, 0);

    gl.glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, rt.color_buffer, 0
    );

    gl.glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    return rt.self_id;
}

Vector2I GLESDriver::render_target_get_size(ResourceID rtid)
{
    return render_targets_get(rtid).size;
}

u32 GLESDriver::make_texture_and_fill(i32 minf, i32 magf, i32 wrap, Vector2I size, i32 internal_format, void* pixels, i32 input_format, i32 target)
{
    u32 gl_id = 0;
    gl.glGenTextures(1, &gl_id);
    gl.glBindTexture(target, gl_id);

    gl.glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minf);
    gl.glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magf);
    gl.glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap);
    gl.glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap);

    gl.glTexImage2D(
        target,
        0,
        internal_format,
        size.width,
        size.height,
        0,
        input_format,
        GL_UNSIGNED_BYTE,
        pixels
    );

    gl.glBindTexture(target, 0);
    return gl_id;
}

GLESDriver::GLESTexture& GLESDriver::textures_allocate()
{
    GLESTexture& texture = data.textures.add(GLESTexture());
    texture.self_id = ResourceID((u32)data.textures.count - 1);
    return texture;
    
}

GLESDriver::GLESTexture& GLESDriver::textures_get(ResourceID rid)
{
    DebugAssert(rid < data.textures.count, "invalid ResourceID");
    return data.textures[rid];
}

GLESDriver::GLESRenderTarget& GLESDriver::render_targets_allocate()
{
    GLESRenderTarget& rt = data.render_targets.add(GLESRenderTarget());
    rt.self_id = ResourceID((u32)data.render_targets.count - 1);
    return rt;
    
}

GLESDriver::GLESRenderTarget& GLESDriver::render_targets_get(ResourceID rid)
{
    DebugAssert(rid < data.render_targets.count, "invalid ResourceID");
    return data.render_targets[rid];
}
