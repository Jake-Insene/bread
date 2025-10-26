#include "graphics/gles/gles_driver.h"

#include "debug/debug.h"
#include "engine/engine.h"
#include "graphics/egl/egl.h"
#include "graphics/gles/gles_vtable.h"
#include "graphics/gles/gles_material_manager.h"
#include "graphics/gles/gles_memory_allocator.h"
#include "graphics/gles/gles_renderer.h"
#include "graphics/viewport.h"


#if SHOW_DEBUG_INFO
static inline void debug_callback(
    GLenum, GLenum, GLuint, GLenum severity,
    GLsizei length, const GLchar* message,
    const void*
)
{
    StringView msg{ message, (usize)length };
    switch(severity)
    {
    case GL_DEBUG_SEVERITY_LOW:
        Log::info("GLInfo: {}\n", msg);
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        Log::warning("GLWarn: {}\n", msg);
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        Log::error("GLError: {}\n", msg);
        break;
    default:
        break;
    }
}
#endif


void GLESDriver::initialize(const mem::Allocator& allocator)
{
    GLESDebugInfo("Initializing renderer...");
    data.allocator = allocator;
    
    EGL::initialize(allocator);

    GLESMemoryAllocator::initialize(allocator);
    GLESMaterialManager::initialize(allocator);
    
    gl.glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &data.limits.max_texture_units);
    gl.glEnable(GL_BLEND);
    gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    _init_context();
}

void GLESDriver::shutdown()
{
    GLESDebugInfo("Shutting down renderer...");
    GLESRenderer::shutdown();
    GLESMaterialManager::shutdown();
    GLESMemoryAllocator::shutdown();
    EGL::shutdown();
}

void GLESDriver::recreate()
{
    EGL::recreate_window_surface();
    const Vector2I new_viewport_size = Engine::get_main_window().get_size();
    data.current_viewport_size = new_viewport_size;
    gl.glViewport(0, 0, new_viewport_size.width, new_viewport_size.height);
    
    GLESDebugInfo("Viewport: W={} H={}", new_viewport_size.width, new_viewport_size.height);
}

void GLESDriver::destroy()
{
    EGL::destroy_window_surface();
}

void GLESDriver::render(Viewport* viewport)
{
    GLESRenderer::render(viewport);
}

void GLESDriver::present(Viewport* viewport)
{
    Vector2I size = Engine::get_main_window().get_size();
    
    auto& rt = GLESMemoryAllocator::render_target_get(viewport->rt.render_target_id);
    // GLES Backbuffer
    if (rt.framebuffer == 0)
    {
        EGL::present();
        return;
    }

    gl.glBindFramebuffer(GL_FRAMEBUFFER, 0);
    gl.glViewport(0, 0, size.width, size.height);

    gl.glBindFramebuffer(GL_READ_FRAMEBUFFER, rt.framebuffer);

    gl.glBlitFramebuffer(
        0, 0, rt.size.width, rt.size.height,
        0, 0, size.width, size.height,
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST
    );

    gl.glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    EGL::present();
}

TextureID GLESDriver::create_texture(const TextureCreateInfo& create_info)
{
    return GLESMemoryAllocator::allocate_texture_from_info(create_info);
}

void GLESDriver::destroy_texture(TextureID tex_id)
{
    GLESMemoryAllocator::texture_free(tex_id);
}

RenderTargetID GLESDriver::create_render_target(const RenderTargetCreateInfo& create_info)
{
    return GLESMemoryAllocator::allocate_render_target_from_info(create_info);
}

void GLESDriver::destroy_render_target(RenderTargetID rt_id)
{
    GLESMemoryAllocator::render_target_free(rt_id);
}

MaterialID GLESDriver::create_material(const MaterialCreateInfo& create_info)
{
    return GLESMaterialManager::create_material(create_info);
}

void GLESDriver::destroy_material(MaterialID material_id)
{
    GLESMaterialManager::destroy_material(material_id);
}

RenderTargetID GLESDriver::get_main_render_target()
{
    // Reserved by GLESMemoryAllocator
    return RenderTargetID(0);
}

void GLESDriver::texture_set_image(TextureID tex_id, Image* img)
{
    GLESMemoryAllocator::texture_set_image(tex_id, img);
}

Vector2I GLESDriver::texture_get_size(TextureID tex_id)
{
    return GLESMemoryAllocator::texture_get_size(tex_id);
}

void GLESDriver::render_target_set_size(RenderTargetID rt_id, const Vector2I& new_size)
{
    GLESMemoryAllocator::render_target_set_size(rt_id, new_size);
}

Vector2I GLESDriver::render_target_get_size(RenderTargetID rt_id)
{
    return GLESMemoryAllocator::render_target_get_size(rt_id);
}

void GLESDriver::material_compile_from_file(MaterialID material_id, StringView path, StringView defines)
{
    GLESMaterialManager::material_compile_from_file(material_id, path, defines);
}

void GLESDriver::material_compile_from_source(MaterialID material_id, StringView source, StringView defines)
{
    GLESMaterialManager::material_compile_from_source(material_id, source, defines);
}

void GLESDriver::_init_context()
{
    // Check openGL on the system
    // Debugging
    Vector2I size = Engine::get_main_window().get_size();

#if SHOW_DEBUG_INFO
    GLint opengl_info[] = { GL_VENDOR, GL_RENDERER, GL_VERSION };
    for (auto name : opengl_info)
    {
        const char* str = (const char*)gl.glGetString(name);
        StringView info = StringView(str, __string_len(str));
        GLESDebugInfo("OpenGL Info: {}", info);
    }

    GLint num_extensions = 0;
    gl.glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    GLESDebugInfo("OpenGL Extensions: {}", num_extensions);
    for (GLint i = 0; i < num_extensions; i++)
    {
        const char* str = (const char*)gl.glGetStringi(GL_EXTENSIONS, i);
        StringView extension = StringView(str, __string_len(str));
        GLESDebugInfo("{}", extension);
    }

#if DEBUG
    if(gl.glDebugMessageCallback && gl.glDebugMessageControl)
    {
        gl.glEnable(GL_DEBUG_OUTPUT);
        gl.glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // Immediate debug messages
        gl.glDebugMessageCallback(debug_callback, nullptr);
        gl.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
#endif

    GLESDebugInfo("Texture Units: {}", data.limits.max_texture_units);
    GLESDebugInfo("Viewport: W={} H={}", size.x, size.y);
#endif

    GLESRenderer::initialize(data.allocator);
}

