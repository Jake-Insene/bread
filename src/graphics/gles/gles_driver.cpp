#include "graphics/gles/gles_driver.h"

#include "engine/engine.h"
#include "external/glcore.h"
#include "graphics/egl/egl.h"
#include "graphics/gles/gles_vtable.h"


#if SHOW_DEBUG_INFO
static inline void debug_callback(
    GLenum, GLenum, GLuint, GLenum severity,
    GLsizei length, const GLchar* message,
    const void*
)
{
    StringView msg = StringView(message, static_cast<usize>(length));
    switch (severity)
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


static inline GLenum _buffer_get_target(Graphics::BufferUsage usage)
{
    switch (usage)
    {
    case Graphics::BUFFER_USAGE_VERTEX:
        return GL_ARRAY_BUFFER;
    case Graphics::BUFFER_USAGE_INDEX:
        return GL_ELEMENT_ARRAY_BUFFER;
    case Graphics::BUFFER_USAGE_UNIFORM:
        return GL_UNIFORM_BUFFER;
    default:
        break;
    }
 
    GLESFailOn(true, "invalid buffer usage");
}

static inline GLenum _buffer_get_usage(Graphics::UploadRate upload_rate, Graphics::MemoryAccess access)
{
    if (upload_rate == Graphics::UPLOAD_RATE_STATIC && access == Graphics::MEMORY_ACCESS_READ_ONLY)
    {
        return GL_STATIC_DRAW;
    }
    else if (upload_rate == Graphics::UPLOAD_RATE_DYNAMIC && access == Graphics::MEMORY_ACCESS_WRITE_ONLY)
    {
        return GL_DYNAMIC_DRAW;
    }

    GLESFailOn(true, "invalid buffer configuration");
}

static inline GLenum _texture_get_format(Graphics::TextureFormat format)
{
    switch (format)
    {
    case Graphics::TEXTURE_FORMAT_RGBA8:
        return GL_RGBA;
    case Graphics::TEXTURE_FORMAT_RGB8:
        return GL_RGB;
    case Graphics::TEXTURE_FORMAT_R8:
        return GL_RED;
    default:
        break;
    }
    GLESFailOn(true, "invalid texture format");
}

static inline GLenum _texture_get_internal_format(Graphics::TextureFormat format)
{
    switch (format)
    {
    case Graphics::TEXTURE_FORMAT_RGBA8:
        return GL_RGBA8;
    case Graphics::TEXTURE_FORMAT_RGB8:
        return GL_RGB8;
    case Graphics::TEXTURE_FORMAT_R8:
        return GL_R8;
    default:
        break;
    }
    GLESFailOn(true, "invalid texture format");
}

static inline GLenum _texture_get_filter(Graphics::TextureFilter filter)
{
    switch (filter)
    {
    case Graphics::TEXTURE_FILTER_LINEAR:
        return GL_LINEAR;
    case Graphics::TEXTURE_FILTER_NEAREST:
        return GL_NEAREST;
    default:
        break;
    }
    GLESFailOn(true, "invalid texture filter");
}


static inline GLenum _render_target_get_format(Graphics::TextureFormat format)
{
    switch (format)
    {
    case Graphics::TEXTURE_FORMAT_RGBA8:
        return GL_RGBA;
    case Graphics::TEXTURE_FORMAT_RGB8:
        return GL_RGB;
    default:
        break;
    }

    GLESFailOn(true, "invalid swap chain format");
}

static inline GLenum _render_target_get_internal_format(Graphics::TextureFormat format)
{
    switch (format)
    {
        case Graphics::TEXTURE_FORMAT_RGBA8:
            return GL_RGBA8;
        case Graphics::TEXTURE_FORMAT_RGB8:
            return GL_RGB8;
        default:
            break;
    }

    GLESFailOn(true, "invalid swap chain format");
}


static inline GLenum _pipeline_get_gl_topology(Graphics::PrimitiveTopology topology)
{
    switch (topology)
    {
    case Graphics::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
        return GL_TRIANGLES;
    case Graphics::PRIMITIVE_TOPOLOGY_LINE_LIST:
        return GL_LINES;
    default:
        break;
    }

    GLESFailOn(true, "invalid primitive topology");
}

static inline i32 _vertex_get_component_count(Graphics::VertexFormat format)
{
    switch (format)
    {
    case Graphics::VERTEX_FORMAT_RGBA32F:
        return 4;
    default:
        break;
    }

    GLESFailOn(true, "invalid vertex format");
}

static inline GLenum _vertex_get_component_type(Graphics::VertexFormat format)
{
    switch (format)
    {
    case Graphics::VERTEX_FORMAT_RGBA32F:
        return GL_FLOAT;
    default:
        break;
    }

    GLESFailOn(true, "invalid vertex format");
}

static inline GLenum _program_get_shader_type(Graphics::ShaderStage stage)
{
    switch (stage)
    {
    case Graphics::SHADER_STAGE_VERTEX:
        return GL_VERTEX_SHADER;
    case Graphics::SHADER_STAGE_FRAGMENT:
        return GL_FRAGMENT_SHADER;
    default:
        break;
    }

    GLESFailOn(true, "invalid shader stage");
}

static inline StringView _program_get_shader_name(Graphics::ShaderStage stage)
{
    switch (stage)
    {
    case Graphics::SHADER_STAGE_VERTEX:
        return "vertex";
    case Graphics::SHADER_STAGE_FRAGMENT:
        return "fragment";
    default:
        break;
    }

    GLESFailOn(true, "invalid shader stage");
}

/*
*   Command Buffer
*/
GLESDriver::CommandBuffer::CommandUnit& GLESDriver::CommandBuffer::allocate()
{
    GLESFailOn(offset == commands.len, "command buffer full!");
    GLESFailOn(ended == true, "command buffer can't record more commands!");
    return commands[offset++];
}


Adapter GLESDriver::get_adapter()
{
    return Adapter
    {
        .initialize = &GLESDriver::initialize,
        .shutdown = &GLESDriver::shutdown,
        .swap_chain_create = &GLESDriver::swap_chain_create,
        .swap_chain_destroy = &GLESDriver::swap_chain_destroy,
        .swap_chain_get_render_target = &GLESDriver::swap_chain_get_render_target,
        .swap_chain_present = &GLESDriver::swap_chain_present,
        .buffer_create = &GLESDriver::buffer_create,
        .buffer_destroy = &GLESDriver::buffer_destroy,
        .buffer_map_memory = &GLESDriver::buffer_map_memory,
        .buffer_unmap_memory = &GLESDriver::buffer_unmap_memory,
        .texture_create = &GLESDriver::texture_create,
        .texture_destroy = &GLESDriver::texture_destroy,
        .texture_get_size = &GLESDriver::texture_get_size,
        .render_target_create = &GLESDriver::render_target_create,
        .render_target_destroy = &GLESDriver::render_target_destroy,
        .render_target_get_texture = &GLESDriver::render_target_get_texture,
        .pipeline_create = &GLESDriver::pipeline_create,
        .pipeline_destroy = &GLESDriver::pipeline_destroy,
        .program_create = &GLESDriver::program_create,
        .program_destroy = &GLESDriver::program_destroy,
        .command_buffer_create = &GLESDriver::command_buffer_create,
        .command_buffer_destroy = &GLESDriver::command_buffer_destroy,
        .command_buffer_begin = &GLESDriver::command_buffer_begin,
        .command_buffer_blit_framebuffer = &GLESDriver::command_buffer_blit_framebuffer,
        .command_buffer_bind_vertex_buffers = &GLESDriver::command_buffer_bind_vertex_buffers,
        .command_buffer_bind_index_buffer = &GLESDriver::command_buffer_bind_index_buffer,
        .command_buffer_bind_pipeline = &GLESDriver::command_buffer_bind_pipeline,
        .command_buffer_bind_render_target = &GLESDriver::command_buffer_bind_render_target,
        .command_buffer_set_texture_unit = &GLESDriver::command_buffer_set_texture_unit,
        .command_buffer_set_uniform = &GLESDriver::command_buffer_set_uniform,
        .command_buffer_set_viewport = &GLESDriver::command_buffer_set_viewport,
        .command_buffer_clear = &GLESDriver::command_buffer_clear,
        .command_buffer_draw = &GLESDriver::command_buffer_draw,
        .command_buffer_draw_indexed = &GLESDriver::command_buffer_draw_indexed,
        .command_buffer_end = &GLESDriver::command_buffer_end,
        .queue_create = &GLESDriver::queue_create,
        .queue_destroy = &GLESDriver::queue_destroy,
        .queue_execute_command_buffer = &GLESDriver::queue_execute_command_buffer,
    };
}

void GLESDriver::initialize(const mem::Allocator& allocator)
{
    GLESDebugInfo("Initializing renderer...");
    data.allocator = allocator;
    
    data.state.draw_framebuffer = 0;
    data.state.read_framebuffer = 0;
    data.state.current_pipeline = Graphics::PipelineID::invalid();
    data.state.gl_index_type = 0;
    data.state.gl_topology = 0;

    EGL::initialize(allocator);

    gl.glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &data.info.max_texture_units);
    
    gl.glEnable(GL_BLEND);
    gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    _init_context();

    data.swapchains = FreeList<SwapChain, Graphics::SwapChainID>::with_size(allocator, 4);
    data.buffers = FreeList<Buffer, Graphics::BufferID>::with_size(allocator, 4);
    data.textures = FreeList<Texture, Graphics::TextureID>::with_size(allocator, 4);
    data.render_targets = FreeList<RenderTarget, Graphics::RenderTargetID>::with_size(allocator, 4);
    data.pipelines = FreeList<Pipeline, Graphics::PipelineID>::with_size(allocator, 4);
    data.programs = FreeList<Program, Graphics::ProgramID>::with_size(allocator, 4);
    data.command_buffers = FreeList<CommandBuffer, Graphics::CommandBufferID>::with_size(allocator, 4);
    data.queues = FreeList<Queue, Graphics::QueueID>::with_size(allocator, 4);
}

void GLESDriver::shutdown()
{
    GLESDebugInfo("Shutting down renderer...");
    EGL::shutdown();

    data.swapchains.destroy();
    data.buffers.destroy();
    data.textures.destroy();
    data.render_targets.destroy();
    data.pipelines.destroy();
    data.programs.destroy();
    data.command_buffers.destroy();
    data.queues.destroy();
}

Graphics::SwapChainID GLESDriver::swap_chain_create(const Graphics::SwapChainCreateInfo& ci)
{
    GLESFailOn(
        ci.image_count != 2 && ci.image_count != 3, 
        "invalid swap chain image count '{}', only 2 or 3 are allowed for now", ci.image_count);
    GLESFailOn(ci.format == Graphics::TEXTURE_FORMAT_UNKNOWN, "invalid swap chain format");
    GLESFailOn(ci.present_mode == Graphics::PRESENT_MODE_UNKNOWN, "invalid swap chain present mode");
    GLESFailOn(ci.size == Vector2I(0, 0), "invalid swap chain image size");
#if !defined(BREAD_ANDROID)
    GLESFailOn(ci.window == Display::WindowID::invalid(), "invalid window id");
#endif

    Graphics::SwapChainID sc_id = data.swapchains.add(SwapChain());
    SwapChain& sc = data.swapchains.get(sc_id);

    sc.window = ci.window;
    sc.images = StaticArray<SwapChainImage, 3>::with_count(ci.image_count);
    
    sc.format = ci.format;
    sc.present_mode = ci.present_mode;
    sc.size = ci.size;

    for (SwapChainImage& image : sc.images.iter())
    {
        Graphics::RenderTargetCreateInfo rtci =
        {
            .format = ci.format,
            .depth_stencil_format = Graphics::TEXTURE_FORMAT_UNKNOWN,
            .size = ci.size,
        };
        image.render_target = render_target_create(rtci);
    }

    return sc_id;
}

void GLESDriver::swap_chain_destroy(Graphics::SwapChainID swap_chain)
{
    SwapChain& sc = data.swapchains.get(swap_chain);
    for (SwapChainImage& image : sc.images.iter())
    {
        render_target_destroy(image.render_target);
    }

    data.swapchains.remove(swap_chain);
}

Graphics::RenderTargetID GLESDriver::swap_chain_get_render_target(Graphics::SwapChainID swap_chain, usize render_target_index)
{
    SwapChain& sc = data.swapchains.get(swap_chain);

    GLESFailOn(render_target_index >= sc.images.count, "invalid render target index");

    return sc.images.get(render_target_index).render_target;
}

void GLESDriver::swap_chain_present(Graphics::SwapChainID swap_chain, usize render_target_index)
{
    SwapChain& sc = data.swapchains.get(swap_chain);
    RenderTarget& rt = data.render_targets.get(
        sc.images.get(render_target_index).render_target
    );
    Vector2I window_size = Display::window_get_size(sc.window);

    if(data.state.draw_framebuffer != 0)
    {
        gl.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        data.state.draw_framebuffer = 0;
    }

    if (data.state.read_framebuffer != rt.glid)
    {
        gl.glBindFramebuffer(GL_READ_FRAMEBUFFER, rt.glid);
        data.state.read_framebuffer = rt.glid;
    }

    gl.glBlitFramebuffer(
        0, 0, rt.size.width, rt.size.height,
        0, 0, window_size.width, window_size.height,
        GL_COLOR_BUFFER_BIT, GL_NEAREST
    );

    EGL::present();
}

Graphics::BufferID GLESDriver::buffer_create(const Graphics::BufferCreateInfo& ci)
{
    GLESFailOn(ci.usage == Graphics::BUFFER_USAGE_UNKNOWN, "invalid buffer usage");
    GLESFailOn(ci.upload_rate == Graphics::UPLOAD_RATE_UNKNOWN, "invalid buffer upload rate");
    GLESFailOn(ci.access == Graphics::MEMORY_ACCESS_UNKNOWN, "invalid buffer memory access");
    GLESFailOn(ci.data.len == 0 && ci.data.ptr() != nullptr, "invalid data, if ptr() is not null len must be greater that zero");
    GLESFailOn(ci.data.len == 0 && ci.data.ptr() == nullptr, "invalid data, len must be greater than zero");

    GLESFailOn(ci.upload_rate == Graphics::UPLOAD_RATE_STATIC && ci.access != Graphics::MEMORY_ACCESS_READ_ONLY, "a inmutable buffer must be declared read only");

    Graphics::BufferID buffer_id = _allocate_buffer();
    Buffer& buffer = data.buffers.get(buffer_id);

    buffer.usage = ci.usage;
    buffer.upload_rate = ci.upload_rate;
    buffer.access = ci.access;

    buffer.target = _buffer_get_target(ci.usage);
    GLenum gl_usage = _buffer_get_usage(ci.upload_rate, ci.access);

    gl.glBindBuffer(buffer.target, buffer.glid);
    gl.glBufferData(
        buffer.target, ci.data.len, ci.data.ptr(), gl_usage
    );
    gl.glBindBuffer(buffer.target, 0);

    return buffer_id;
}

void GLESDriver::buffer_destroy(Graphics::BufferID buffer)
{
    Buffer& bf = data.buffers.get(buffer);
    gl.glDeleteBuffers(1, &bf.glid);
    data.buffers.remove(buffer);
}

Slice<u8> GLESDriver::buffer_map_memory(Graphics::BufferID buffer, usize offset, usize len)
{
    Buffer& bf = data.buffers.get(buffer);

    GLbitfield access = 0;
    if (bf.access == Graphics::MEMORY_ACCESS_WRITE_ONLY || bf.access == Graphics::MEMORY_ACCESS_READ_WRITE)
    {
        access = GL_MAP_WRITE_BIT;
    }

    gl.glBindBuffer(bf.target, bf.glid);
    void* memory = gl.glMapBufferRange(bf.target, offset, len, access);
    gl.glBindBuffer(bf.target, 0);

    return Slice<u8>(reinterpret_cast<u8*>(memory), len);
}

void GLESDriver::buffer_unmap_memory(Graphics::BufferID buffer, const Slice<u8>& memory)
{
    Unused(memory);
    Buffer& bf = data.buffers.get(buffer);
    gl.glBindBuffer(bf.target, bf.glid);
    gl.glUnmapBuffer(bf.target);
    gl.glBindBuffer(bf.target, 0);
}

Graphics::TextureID GLESDriver::texture_create(const Graphics::TextureCreateInfo& ci)
{
    GLESFailOn(ci.usage == Graphics::TEXTURE_USAGE_UNKNOWN, "invalid texture usage");
    GLESFailOn(ci.type == Graphics::TEXTURE_UNKNOWN, "invalid texture type");
    GLESFailOn(ci.format == Graphics::TEXTURE_FORMAT_UNKNOWN, "invalid texture format");
    GLESFailOn(ci.min_filter == Graphics::TEXTURE_FILTER_UNKNOWN, "invalid texture min filter");
    GLESFailOn(ci.mag_filter == Graphics::TEXTURE_FILTER_UNKNOWN, "invalid texture mag filter");

    Graphics::TextureID texture_id = _allocate_texture();
    Texture& texture = data.textures.get(texture_id);
    texture.size = ci.size;

    gl.glBindTexture(GL_TEXTURE_2D, texture.glid);

    GLenum format = _texture_get_format(ci.format);
    GLenum internal_format = _texture_get_internal_format(ci.format);

    gl.glTexImage2D(
        GL_TEXTURE_2D, 0, internal_format,
        ci.size.width, ci.size.height, 0, 
        format, GL_UNSIGNED_BYTE, ci.pixels.ptr()
    );

    GLenum min_filter = _texture_get_filter(ci.min_filter);
    GLenum mag_filter = _texture_get_filter(ci.mag_filter);

    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    gl.glBindTexture(GL_TEXTURE_2D, 0);

    return texture_id;
}

void GLESDriver::texture_destroy(Graphics::TextureID texture)
{
    Texture& tex = data.textures.get(texture);
    gl.glDeleteTextures(1, &tex.glid);
    data.textures.remove(texture);
}

Vector2I GLESDriver::texture_get_size(Graphics::TextureID texture)
{
    Texture& tex = data.textures.get(texture);
    return tex.size;
}

Graphics::RenderTargetID GLESDriver::render_target_create(const Graphics::RenderTargetCreateInfo& ci)
{
    GLESFailOn(ci.format == Graphics::TEXTURE_FORMAT_UNKNOWN, "invalid render target format");
    GLESFailOn(ci.size == Vector2I(0, 0), "invalid render target size");

    Graphics::RenderTargetID rt_id = _allocate_render_target();
    RenderTarget& rt = data.render_targets.get(rt_id);

    rt.format = ci.format;
    rt.size = ci.size;

    gl.glBindFramebuffer(GL_FRAMEBUFFER, rt.glid);
    GLenum gl_format = _render_target_get_format(ci.format);
    GLenum gl_internal_format = _render_target_get_internal_format(ci.format);

    { // FRAMEBUFFER TEXTURE

        rt.texture = _allocate_texture();
        Texture& texture = data.textures.get(rt.texture);
        texture.size = ci.size;

        gl.glBindTexture(GL_TEXTURE_2D, texture.glid);
        gl.glTexImage2D(
            GL_TEXTURE_2D, 0, gl_internal_format,
            ci.size.width, ci.size.height, 0,
            gl_format, GL_UNSIGNED_BYTE, nullptr
        );
        gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        gl.glBindTexture(GL_TEXTURE_2D, 0);

        gl.glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
            texture.glid, 0
        );

    }

    GLenum framebuffer_state = gl.glCheckFramebufferStatus(GL_FRAMEBUFFER);

    GLESFailOn(
            framebuffer_state != GL_FRAMEBUFFER_COMPLETE,
            "the framebuffer was not created correctly"
    );

    gl.glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return rt_id;
}

void GLESDriver::render_target_destroy(Graphics::RenderTargetID render_target)
{
    RenderTarget& rt = data.render_targets.get(render_target);
    gl.glDeleteFramebuffers(1, &rt.glid);
    texture_destroy(rt.texture);
    data.render_targets.remove(render_target);
}

Graphics::TextureID GLESDriver::render_target_get_texture(Graphics::RenderTargetID render_target)
{
    RenderTarget& rt = data.render_targets.get(render_target);
    return rt.texture;
}

Graphics::PipelineID GLESDriver::pipeline_create(const Graphics::PipelineCreateInfo& ci)
{
    GLESFailOn(ci.usage == Graphics::PIPELINE_USAGE_UNKNOWN, "invalid pipeline usage");
    GLESFailOn(ci.topology == Graphics::PRIMITIVE_TOPOLOGY_UNKNOWN, "invalid primitive topology");
    GLESFailOn(ci.input_assembly.bindings.len == 0 && ci.input_assembly.attributes.len != 0, 
        "invalid input assembly, if attributes is not zero bindings must be at least one");

    Graphics::PipelineID pipeline_id = data.pipelines.add(Pipeline());
    Pipeline& pipeline = data.pipelines.get(pipeline_id);

    pipeline.vertex_array = 0;
    pipeline.gl_topology = _pipeline_get_gl_topology(ci.topology);
    pipeline.gl_program = 0;

    pipeline.usage = ci.usage;
    pipeline.topology = ci.topology;
    pipeline.program = ci.pipeline_program;

    if (pipeline.program != Graphics::ProgramID::invalid())
    {
        Program& prog = data.programs.get(pipeline.program);
        pipeline.gl_program = prog.glid;
    }

    gl.glGenVertexArrays(1, &pipeline.vertex_array);
    if (ci.input_assembly.bindings.len)
    {
        gl.glBindVertexArray(pipeline.vertex_array);

        for (Graphics::VertexBinding& binding : ci.input_assembly.bindings)
        {
            gl.glBindVertexBuffer(
                binding.binding,
                0,
                0,
                binding.stride
            );
        }

        for (Graphics::VertexAttribute& attribute : ci.input_assembly.attributes)
        {
            gl.glVertexAttribFormat(
                attribute.location, 
                _vertex_get_component_count(attribute.format),
                _vertex_get_component_type(attribute.format),
                GL_FALSE, attribute.offset
            );
            // TODO: enable multiple bindings
            if(ci.input_assembly.bindings[0].input_rate == Graphics::INPUT_RATE_PER_INSTANCE)
            {
                gl.glVertexAttribDivisor(attribute.location, 1);
            }

            gl.glVertexAttribBinding(attribute.location, attribute.binding);
            
            gl.glEnableVertexAttribArray(attribute.location);
        }

        gl.glBindVertexArray(0);
    }

    return pipeline_id;
}

void GLESDriver::pipeline_destroy(Graphics::PipelineID pipeline)
{
    Pipeline& pipe = data.pipelines.get(pipeline);
    if (pipe.vertex_array != 0)
    {
        gl.glDeleteVertexArrays(1, &pipe.vertex_array);
    }

    data.pipelines.remove(pipeline);
}

Graphics::ProgramID GLESDriver::program_create(const Graphics::ProgramCreateInfo& ci)
{
    GLESFailOn(ci.shaders.len == 0, "at least one shader was expected");

    Graphics::ProgramID program_id = data.programs.add(Program());
    Program& program = data.programs.get(program_id);

    program.glid = gl.glCreateProgram();

    StaticArray<GLID, 10> gl_shaders = StaticArray<GLID, 10>::with_count(0);
    StringView source_path;

    for (Graphics::ShaderInfo& shader : ci.shaders)
    {
        GLESFailOn(shader.stage == Graphics::SHADER_STAGE_UNKNOWN, "invalid shader stage");

        GLID sh = gl.glCreateShader(_program_get_shader_type(shader.stage));
        (void)gl_shaders.add(sh);

        GLint length = static_cast<GLint>(shader.code.len);
        const GLchar* source = reinterpret_cast<const GLchar*>(shader.code.items);

        gl.glShaderSource(sh, 1, &source, &length);
        gl.glCompileShader(sh);

        i32 status = GL_TRUE;
        char log[512] = {};

        gl.glGetShaderiv(sh, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE)
        {
            i32 len = 0;
            gl.glGetShaderInfoLog(sh, 512, &len, log);
            StringView log_view = StringView(log, static_cast<usize>(len));
            GLESFatal(
                "Error compiling the {} shader: '{}':\n{}", 
                _program_get_shader_name(shader.stage),
                shader.source_path.len ? shader.source_path : "__source__",
                log_view
            );
        }

        gl.glAttachShader(program.glid, sh);

        source_path = shader.source_path;
    }

    gl.glLinkProgram(program.glid);

    i32 status = GL_TRUE;
    char log[512] = {};

    gl.glGetProgramiv(program.glid, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        i32 len = 0;
        gl.glGetProgramInfoLog(program.glid, 512, &len, log);
        StringView log_view = StringView(log, static_cast<usize>(len));

        GLESFatal(
            "Error linking the shader program: '{}':\n{}", 
            source_path.len ? source_path : "__source__",
            log_view
        );
    }

    for (GLID& sh : gl_shaders.iter())
    {
        gl.glDeleteShader(sh);
    }

    return program_id;
}

void GLESDriver::program_destroy(Graphics::ProgramID program)
{
    Program& prog = data.programs.get(program);
    gl.glDeleteProgram(prog.glid);
    data.programs.remove(program);
}

Graphics::CommandBufferID GLESDriver::command_buffer_create(const Graphics::CommandBufferCreateInfo& ci)
{
    GLESFailOn(ci.usage == Graphics::COMMAND_BUFFER_USAGE_UNKNOWN, "invalid command buffer usage");

    Graphics::CommandBufferID cb_id = data.command_buffers.add(CommandBuffer());
    CommandBuffer& cb = data.command_buffers.get(cb_id);
    cb.usage = ci.usage;
    cb.commands = get_allocator().array<CommandBuffer::CommandUnit>(DefaultCommandBufferSize);
    cb.offset = 0;

    return cb_id;
}

void GLESDriver::command_buffer_destroy(Graphics::CommandBufferID cmd)
{
    CommandBuffer& cb = data.command_buffers.get(cmd);
    get_allocator().free(mem::to_bytes(cb.commands));

    data.command_buffers.remove(cmd);
}

void GLESDriver::command_buffer_begin(Graphics::CommandBufferID cmd)
{
    CommandBuffer& cb = data.command_buffers.get(cmd);
    cb.offset = 0;
    cb.ended = false;
}

void GLESDriver::command_buffer_blit_framebuffer(Graphics::CommandBufferID cmd, Graphics::RenderTargetID src_render_target, Graphics::RenderTargetID dst_render_target, Rect2DI src_rect, Rect2DI dst_rect, Graphics::TextureFilter filter)
{
    CommandBuffer& cb = data.command_buffers.get(cmd);
    CommandBuffer::CommandUnit& command = cb.allocate();
    command.base.type = CommandBuffer::COMMAND_TYPE_BLIT_FRAMEBUFFER;
    command.blit_framebuffer.src_render_target = src_render_target;
    command.blit_framebuffer.dst_render_target = dst_render_target;
    command.blit_framebuffer.src_rect = src_rect;
    command.blit_framebuffer.dst_rect = dst_rect;
    command.blit_framebuffer.filter = filter;
}


void GLESDriver::command_buffer_bind_vertex_buffers(Graphics::CommandBufferID cmd, u32 binding, const Slice<Graphics::BufferID>& buffers, const Slice<u32>& offsets, const Slice<u32>& strides)
{
    GLESFailOn(buffers.len != offsets.len, "buffer and offset count doesn't match");
    GLESFailOn(buffers.len != strides.len, "buffer and stride count doesn't match");

    CommandBuffer& cb = data.command_buffers.get(cmd);
    CommandBuffer::CommandUnit& command = cb.allocate();
    command.base.type = CommandBuffer::COMMAND_TYPE_BIND_VERTEX_BUFFER;
    command.bind_vertex_buffer.binding = binding;
    command.bind_vertex_buffer.binding_count = static_cast<u32>(buffers.len);

    for (usize i = 0; i < buffers.len; i++)
    {
        command.bind_vertex_buffer.bindings[i].buffer = buffers[i];
        command.bind_vertex_buffer.bindings[i].offset = offsets[i];
        command.bind_vertex_buffer.bindings[i].stride = strides[i];
    }
}

void GLESDriver::command_buffer_bind_index_buffer(Graphics::CommandBufferID cmd, Graphics::BufferID index_buffer, u32 offset, Graphics::IndexType index_type)
{
    CommandBuffer& cb = data.command_buffers.get(cmd);
    CommandBuffer::CommandUnit& command = cb.allocate();
    command.base.type = CommandBuffer::COMMAND_TYPE_BIND_INDEX_BUFFER;
    command.bind_index_buffer.index_buffer = index_buffer;
    command.bind_index_buffer.offset = offset;
    command.bind_index_buffer.index_type = index_type;
}

void GLESDriver::command_buffer_bind_pipeline(Graphics::CommandBufferID cmd, Graphics::PipelineID pipeline)
{
    CommandBuffer& cb = data.command_buffers.get(cmd);
    CommandBuffer::CommandUnit& command = cb.allocate();
    command.base.type = CommandBuffer::COMMAND_TYPE_BIND_PIPELINE;
    command.bind_pipeline.pipeline = pipeline;
}

void GLESDriver::command_buffer_bind_render_target(Graphics::CommandBufferID cmd, Graphics::RenderTargetID render_target)
{
    CommandBuffer& cb = data.command_buffers.get(cmd);
    CommandBuffer::CommandUnit& command = cb.allocate();
    command.base.type = CommandBuffer::COMMAND_TYPE_BIND_RENDER_TARGET;
    command.bind_render_target.render_target = render_target;
}

void GLESDriver::command_buffer_set_texture_unit(Graphics::CommandBufferID cmd, u32 set, u32 base_slot, const Slice<Graphics::TextureID>& textures)
{
    GLESFailOn(textures.len > 16, "only 16 texture units are supported by now");

    CommandBuffer& cb = data.command_buffers.get(cmd);
    CommandBuffer::CommandUnit& command = cb.allocate();
    command.base.type = CommandBuffer::COMMAND_TYPE_SET_TEXTURE_UNIT;
    command.set_texture_unit.set = set;
    command.set_texture_unit.base_slot = base_slot;
    command.set_texture_unit.texture_count = static_cast<u32>(textures.len);
    for (usize i = 0; i < textures.len; i++)
    {
        command.set_texture_unit.textures[i] = textures[i];
    }
}

void GLESDriver::command_buffer_set_uniform(Graphics::CommandBufferID cmd, u32 set, u32 base_slot, const Slice<Graphics::BufferID>& buffers)
{
    GLESFailOn(buffers.len > 8, "only 16 uniform buffer units are supported by now");

    CommandBuffer& cb = data.command_buffers.get(cmd);
    CommandBuffer::CommandUnit& command = cb.allocate();
    command.base.type = CommandBuffer::COMMAND_TYPE_SET_UNIFORM;
    command.set_uniform.set = set;
    command.set_uniform.base_slot = base_slot;
    command.set_uniform.uniform_count = static_cast<u32>(buffers.len);
    for (usize i = 0; i < buffers.len; i++)
    {
        command.set_uniform.uniforms[i] = buffers[i];
    }
}

void GLESDriver::command_buffer_set_viewport(Graphics::CommandBufferID cmd, Rect2DI viewport_rect)
{
    CommandBuffer& cb = data.command_buffers.get(cmd);
    CommandBuffer::CommandUnit& command = cb.allocate();
    command.base.type = CommandBuffer::COMMAND_TYPE_SET_VIEWPORT;
    command.set_viewport.viewport_rect = viewport_rect;
}

void GLESDriver::command_buffer_clear(Graphics::CommandBufferID cmd, Graphics::RenderTargetID render_target, Color clear_color)
{
    CommandBuffer& cb = data.command_buffers.get(cmd);
    CommandBuffer::CommandUnit& command = cb.allocate();
    command.base.type = CommandBuffer::COMMAND_TYPE_CLEAR;
    command.clear.render_target = render_target;
    command.clear.clear_color = clear_color;
}

void GLESDriver::command_buffer_draw(Graphics::CommandBufferID cmd, u32 index_count, u32 instance_count, u32 base_vertex, u32 base_instance)
{
    CommandBuffer& cb = data.command_buffers.get(cmd);
    CommandBuffer::CommandUnit& command = cb.allocate();
    command.base.type = CommandBuffer::COMMAND_TYPE_DRAW;
    command.draw.vertex_count = index_count;
    command.draw.instance_count = instance_count;
    command.draw.base_vertex = base_vertex;
    command.draw.base_instance = base_instance;
}

void GLESDriver::command_buffer_draw_indexed(Graphics::CommandBufferID cmd, u32 index_count, u32 instance_count, u32 base_index, u32 base_vertex, u32 base_instance)
{
    CommandBuffer& cb = data.command_buffers.get(cmd);
    CommandBuffer::CommandUnit& command = cb.allocate();
    command.base.type = CommandBuffer::COMMAND_TYPE_DRAW_INDEXED;
    command.draw_indexed.index_count = index_count;
    command.draw_indexed.instance_count = instance_count;
    command.draw_indexed.base_index = base_index;
    command.draw_indexed.base_vertex = base_vertex;
    command.draw_indexed.base_instance = base_instance;
}

void GLESDriver::command_buffer_end(Graphics::CommandBufferID cmd)
{
    CommandBuffer& cb = data.command_buffers.get(cmd);
    cb.ended = true;
}

Graphics::QueueID GLESDriver::queue_create(const Graphics::QueueCreateInfo& ci)
{
    GLESFailOn(ci.usage == Graphics::QUEUE_USAGE_UNKNOWN, "invalid queue usage");

    Graphics::QueueID queue_id = data.queues.add(Queue());
    Queue& queue = data.queues.get(queue_id);
    queue.usage = ci.usage;
    return queue_id;
}

void GLESDriver::queue_destroy(Graphics::QueueID queue)
{
    data.queues.remove(queue);
}

void GLESDriver::queue_execute_command_buffer(Graphics::QueueID queue, const Slice<Graphics::CommandBufferID>& command_buffers)
{
    Queue& q = data.queues.get(queue);
    
    for (Graphics::CommandBufferID& cb : command_buffers)
    {
        _queue_execute_command_buffer(q, data.command_buffers.get(cb));
    }

    if (data.state.current_pipeline != Graphics::PipelineID::invalid())
    {
        data.state.current_pipeline = Graphics::PipelineID::invalid();
        gl.glBindVertexArray(0);
    }
}

void GLESDriver::_queue_execute_command_buffer(Queue& q, CommandBuffer& command_buffer)
{
    Unused(q);

    for (usize cmd_index = 0; cmd_index < command_buffer.offset; cmd_index++)
    {
        CommandBuffer::CommandUnit& cmd = command_buffer.commands[cmd_index];
        switch (cmd.base.type)
        {
        case CommandBuffer::COMMAND_TYPE_BLIT_FRAMEBUFFER:
        {
            RenderTarget& src_rt = data.render_targets.get(cmd.blit_framebuffer.src_render_target);
            RenderTarget& dst_rt = data.render_targets.get(cmd.blit_framebuffer.dst_render_target);

            GLenum filter = _texture_get_filter(cmd.blit_framebuffer.filter);

            if(data.state.draw_framebuffer != dst_rt.glid)
            {
                gl.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst_rt.glid);
                data.state.draw_framebuffer = dst_rt.glid;
            }

            if (data.state.read_framebuffer != src_rt.glid)
            {
                gl.glBindFramebuffer(GL_READ_FRAMEBUFFER, src_rt.glid);
                data.state.read_framebuffer = src_rt.glid;
            }            

            gl.glBlitFramebuffer(
                cmd.blit_framebuffer.src_rect.position.x,
                cmd.blit_framebuffer.src_rect.position.y,
                cmd.blit_framebuffer.src_rect.size.width,
                cmd.blit_framebuffer.src_rect.size.height,
                cmd.blit_framebuffer.dst_rect.position.x,
                cmd.blit_framebuffer.dst_rect.position.y,
                cmd.blit_framebuffer.dst_rect.size.x,
                cmd.blit_framebuffer.dst_rect.size.y,
                GL_COLOR_BUFFER_BIT,
                filter
            );
        }
            break;
        case CommandBuffer::COMMAND_TYPE_BIND_VERTEX_BUFFER:
        {
            GLESFailOn(data.state.current_pipeline == Graphics::PipelineID::invalid(), "A pipeline is not bounded");

            //Pipeline& pipeline = data.pipelines.get(data.state.current_pipeline);

            for (u32 vbi = 0; vbi < cmd.bind_vertex_buffer.binding_count; vbi++)
            {
                CommandBuffer::VertexBufferBinding vbb = cmd.bind_vertex_buffer.bindings[vbi];
                GLID buffer = data.buffers.get(vbb.buffer).glid;
                u32 offset = vbb.offset;
                u32 stride = vbb.stride;

                gl.glBindVertexBuffer(
                    cmd.bind_vertex_buffer.binding + vbi,
                    buffer,
                    offset,
                    stride
                );
            }
        }
            break;
        case CommandBuffer::COMMAND_TYPE_BIND_INDEX_BUFFER:
        {
            Buffer& buffer = data.buffers.get(cmd.bind_index_buffer.index_buffer);

            gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.glid);
            switch (cmd.bind_index_buffer.index_type)
            {
            case Graphics::INDEX_TYPE_UINT8:
                data.state.gl_index_type = GL_UNSIGNED_BYTE;
                break;
            case Graphics::INDEX_TYPE_UINT16:
                data.state.gl_index_type = GL_UNSIGNED_SHORT;
                break;
            case Graphics::INDEX_TYPE_UINT32:
                data.state.gl_index_type = GL_UNSIGNED_INT;
                break;
            default:
                GLESFailOn(true, "invalid index type");
                break;
            }
        }
            break;
        case CommandBuffer::COMMAND_TYPE_BIND_PIPELINE:
        {
            Pipeline& pipeline = data.pipelines.get(cmd.bind_pipeline.pipeline);
            data.state.current_pipeline = cmd.bind_pipeline.pipeline;
            data.state.gl_topology = pipeline.gl_topology;

            gl.glBindVertexArray(pipeline.vertex_array);
            gl.glUseProgram(pipeline.gl_program);
        }
            break;

        case CommandBuffer::COMMAND_TYPE_BIND_RENDER_TARGET:
        {
            RenderTarget& render_target = data.render_targets.get(cmd.bind_render_target.render_target);
            if (data.state.draw_framebuffer != render_target.glid)
            {
                gl.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, render_target.glid);
                data.state.draw_framebuffer = render_target.glid;
            }
        }
        break;
        case CommandBuffer::COMMAND_TYPE_SET_TEXTURE_UNIT:
        {
            for (u32 tex_index = 0; tex_index < cmd.set_texture_unit.texture_count; tex_index++)
            {
                Graphics::TextureID texture_id = cmd.set_texture_unit.textures[tex_index];
                Texture& texture = data.textures.get(texture_id);

                gl.glActiveTexture(GL_TEXTURE0 + cmd.set_texture_unit.base_slot + tex_index);
                gl.glBindTexture(GL_TEXTURE_2D, texture.glid);
            }
        }
            break;
        case CommandBuffer::COMMAND_TYPE_SET_UNIFORM:
        {
            for (u32 uni_index = 0; uni_index < cmd.set_uniform.uniform_count; uni_index++)
            {
                Graphics::BufferID buffer_id = cmd.set_uniform.uniforms[uni_index];
                Buffer& uniform_buffer = data.buffers.get(buffer_id);

                gl.glBindBufferBase(
                    GL_UNIFORM_BUFFER, 
                    cmd.set_uniform.base_slot + uni_index, uniform_buffer.glid
                );
            }
        }
        break;
        case CommandBuffer::COMMAND_TYPE_SET_VIEWPORT:
            gl.glViewport(
                cmd.set_viewport.viewport_rect.position.x,
                cmd.set_viewport.viewport_rect.position.y,
                cmd.set_viewport.viewport_rect.size.width,
                cmd.set_viewport.viewport_rect.size.height
            );
            break;
        case CommandBuffer::COMMAND_TYPE_CLEAR:
        {
            RenderTarget& rt = data.render_targets.get(cmd.clear.render_target);

            if (data.state.draw_framebuffer != rt.glid)
            {
                gl.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rt.glid);
                data.state.draw_framebuffer = rt.glid;
            }

            gl.glClearColor(
                cmd.clear.clear_color.r / 255.f,
                cmd.clear.clear_color.g / 255.f,
                cmd.clear.clear_color.b / 255.f,
                cmd.clear.clear_color.a / 255.f
            );
            gl.glClear(GL_COLOR_BUFFER_BIT);
        }
            break;
        case CommandBuffer::COMMAND_TYPE_DRAW:
            gl.glDrawArraysInstanced(
                data.state.gl_topology, cmd.draw.base_vertex, cmd.draw.vertex_count, cmd.draw.instance_count
            );
            break;
        case CommandBuffer::COMMAND_TYPE_DRAW_INDEXED:
        {
            gl.glDrawElementsInstanced(
                data.state.gl_topology, cmd.draw_indexed.index_count, data.state.gl_index_type,
                nullptr, cmd.draw_indexed.instance_count
            );
        }
            break;
        case CommandBuffer::COMMAND_TYPE_UNKNOWN:
            GLESFailOn(true, "invalid command type");
            break;
        }
    }
}

Graphics::BufferID GLESDriver::_allocate_buffer()
{
    Graphics::BufferID buffer_id = data.buffers.add(Buffer());
    Buffer& buffer = data.buffers.get(buffer_id);
    gl.glGenBuffers(1, &buffer.glid);
    return buffer_id;
}

Graphics::TextureID GLESDriver::_allocate_texture()
{
    Graphics::TextureID texture_id = data.textures.add(Texture());
    Texture& texture = data.textures.get(texture_id);
    gl.glGenTextures(1, &texture.glid);
    return texture_id;
}

Graphics::RenderTargetID GLESDriver::_allocate_render_target()
{
    Graphics::RenderTargetID rt_id = data.render_targets.add(RenderTarget());
    RenderTarget& rt = data.render_targets.get(rt_id);
    gl.glGenFramebuffers(1, &rt.glid);
    return rt_id;
}

void GLESDriver::_init_context()
{
    // Check openGL on the system
    // Debugging
#if SHOW_DEBUG_INFO
    Vector2I size = Engine::get_main_window().get_size();

    GLint opengl_info[] = { GL_VENDOR, GL_RENDERER, GL_VERSION };
    for (GLint name : opengl_info)
    {
        const char* str = reinterpret_cast<const char*>(gl.glGetString(name));
        StringView info = StringView(str, __string_len(str));
        GLESDebugInfo("OpenGL Info: {}", info);
    }

    GLint num_extensions = 0;
    gl.glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    GLESDebugInfo("OpenGL Extensions: {}", num_extensions);
    for (GLint i = 0; i < num_extensions; i++)
    {
        const char* str = reinterpret_cast<const char*>(gl.glGetStringi(GL_EXTENSIONS, i));
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

    GLESDebugInfo("Texture Units: {}", data.info.max_texture_units);
    GLESDebugInfo("Viewport: W={} H={}", size.x, size.y);
#endif
}


