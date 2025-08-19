#include "graphics/gles/gles_driver.h"

#include "debug/debug.h"
#include "engine/engine.h"
#include "graphics/egl/egl.h"
#include "graphics/gles/gles_vtable.h"
#include "graphics/gles/gles_cmd_proc.h"
#include "graphics/gles/gles_memory_allocator.h"


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
        
        .create_texture = &GLESMemoryAllocator::allocate_texture_from_info,
        .destroy_texture = &GLESMemoryAllocator::texture_free,
        .create_render_target = &GLESMemoryAllocator::allocate_render_target_from_info,
        .destroy_render_target = &GLESMemoryAllocator::render_target_free,
        
        .texture_set_image = &GLESMemoryAllocator::texture_set_image,
        .texture_get_size = &GLESMemoryAllocator::texture_get_size,
        
        .render_target_get_size = &GLESMemoryAllocator::render_target_get_size,
        .render_target_set_size = &GLESMemoryAllocator::render_target_set_size,
    };
}

void GLESDriver::initialize(const mem::Allocator& allocator)
{
    GLESDebugInfo("Initializing renderer...");
    data.allocator = allocator;

    GLESMemoryAllocator::initialize(allocator);
    
    EGL::initialize(allocator);
    
    gl.glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &data.limits.max_texture_units);
    gl.glEnable(GL_BLEND);
    gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    _init_context();
}

void GLESDriver::shutdown()
{
    GLESDebugInfo("Shutting down renderer...");
    GLESCommandProcessor::shutdown();
    GLESMemoryAllocator::shutdown();
    EGL::shutdown();
}

void GLESDriver::recreate()
{
    EGL::recreate_window_surface();

    Vector2I size = Engine::get_main_window().get_size();
    GLESCommandProcessor::recreate_window_transform(size);

    gl.glViewport(0, 0, size.x, size.y);
    GLESDebugInfo("Viewport: W={} H={}", size.x, size.y);
}

void GLESDriver::destroy()
{
    EGL::destroy_window_surface();
}

void GLESDriver::render()
{
    GLESCommandProcessor::render();
}

void GLESDriver::present()
{
    Vector2I size = Engine::get_main_window().get_size();
    Graphics::RenderTargetID rt_id = GLESCommandProcessor::get_current_fb();
    
    if (rt_id == InvalidResource)
    {
        EGL::present();
        return;
    }

    auto& rt = GLESMemoryAllocator::render_target_get(GLESCommandProcessor::get_current_fb());
    if (GLESCommandProcessor::data.state.current_fbo != 0)
    {
        gl.glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    gl.glViewport(0, 0, size.width, size.height);

    gl.glBindFramebuffer(GL_READ_FRAMEBUFFER, rt.framebuffer);

    gl.glBlitFramebuffer(
        0, 0, rt.size.width, rt.size.height,
        0, 0, size.width, size.height,
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST
    );

    gl.glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    GLESCommandProcessor::data.state.last_fbo = GLESCommandProcessor::get_current_fbo();
    GLESCommandProcessor::set_current_fbo(0);

    EGL::present();
}

void GLESDriver::add_cmd(const RenderCommand& cmd)
{
    (void)GLESCommandProcessor::data.commands.add(cmd);
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
        const char* info = (const char*)gl.glGetString(name);
        GLESDebugInfo("OpenGL Info: {}", info);
    }

    GLint num_extensions = 0;
    gl.glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    GLESDebugInfo("OpenGL Extensions: {}", num_extensions);
    for (GLint i = 0; i < num_extensions; i++)
    {
        const char* extension = (const char*)gl.glGetStringi(GL_EXTENSIONS, i);
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
    gl.glViewport(0, 0, size.x, size.y );

    GLESCommandProcessor::initialize(data.allocator);
}

