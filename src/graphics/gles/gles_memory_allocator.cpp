#include "graphics/gles/gles_memory_allocator.h"

#include "debug/debug.h"
#include "graphics/gles/gles_vtable.h"
#include "math/values.h"

static inline usize _get_format_size(GLenum internal_format)
{
    switch (internal_format)
    {
    case 0:
        return 0;
    case GL_RGB:
    case GL_RGB8:
        return 3;
    case GL_RGBA:
    case GL_RGBA8:
        return 4;
    case GL_RED:
    case GL_R8:
        return 1;
    default:
        DebugAssert(false, "invalid texture type");
        break;
    }

    return 0;
}


static inline GLenum _get_texture_target(TextureType type)
{
    switch (type)
    {
    case TEXTURE_2D:
        return GL_TEXTURE_2D;
    default:
        DebugAssert(false, "invalid texture type");
        break;
    }

    return 0;
}

static inline GLenum _get_texture_format(TextureFormat format)
{
    switch (format)
    {
    case TEXTURE_FORMAT_RGB8:
        return GL_RGB;
    case TEXTURE_FORMAT_RGBA8:
        return GL_RGBA;
    case TEXTURE_FORMAT_R8:
        return GL_RED;
    default:
        DebugAssert(false, "invalid texture format");
        break;
    }

    return 0;
}

static inline GLenum _get_texture_internal_format_for(GLenum format)
{
    switch (format)
    {
    case GL_RGB:
        return GL_RGB8;
    case GL_RGBA:
        return GL_RGBA8;
    case GL_RED:
        return GL_R8;
    default:
        DebugAssert(false, "invalid texture format");
        break;
    }
    return 0;
}

static inline GLenum _get_texture_filter(TextureFilter filter)
{
    switch (filter)
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

void GLESMemoryAllocator::initialize(const mem::Allocator& allocator)
{
    data.allocator = allocator;

    data.buffers = Array<GLESBuffer>::with_size(allocator, 4);
    data.textures = QueueArray<GLESTexture, ResourceID>::with_size(allocator, 4);
    data.render_targets = QueueArray<GLESRenderTarget, RenderTargetID>::with_size(allocator, 4);

    // Allocating the backbuffer we should present this instead of a intermediate backbuffer.
    GLESRenderTarget& rt = render_target_allocate();
    rt.framebuffer = 0;
}

void GLESMemoryAllocator::shutdown()
{
    GLESDebugInfo(
        "Graphics:\n"
        "\tCurrent Allocated bytes: {}",
        data.allocated_bytes
    );

    // Unload graphics resources.
    for (auto& buffer : data.buffers)
    {
        buffer_free(buffer.self_id);
    }

    data.buffers.destroy();
    data.textures.destroy();
    data.render_targets.destroy();

    DebugAssert(data.allocated_bytes == 0, "Graphics: memory leak!");
}

GLESMemoryAllocator::GLESBuffer& GLESMemoryAllocator::buffer_allocate()
{
    GLESBuffer& buffer = data.buffers.add(GLESBuffer());
    buffer.self_id = data.buffers.count - 1 & math::MaxValue<ResourceID>;
    return buffer;
}

void GLESMemoryAllocator::buffer_free(ResourceID rid)
{
    GLESBuffer& buffer = buffer_get(rid);
    gl.glDeleteBuffers(1, &buffer.buffer);

    data.allocated_bytes -= buffer.size;
}

GLID GLESMemoryAllocator::buffer_allocate_handle()
{
    GLID buffer;
    gl.glGenBuffers(1, &buffer);
    return buffer;
}

void GLESMemoryAllocator::buffer_deallocate_handle(GLID buffer, usize size)
{
    gl.glDeleteBuffers(1, &buffer);
    data.allocated_bytes -= size;
}

void GLESMemoryAllocator::buffer_fill_memory(GLID buffer, Slice<const u8> mem, GLenum target, GLenum usage)
{
    gl.glBindBuffer(target, buffer);
    gl.glBufferData(target, mem.len, mem.ptr(), usage);
    gl.glBindBuffer(target, 0);

    data.allocated_bytes += mem.len;
}

void GLESMemoryAllocator::buffer_update_memory(GLID buffer, usize offset, Slice<const u8> mem, GLenum target)
{
    gl.glBindBuffer(target, buffer);
    gl.glBufferSubData(target, offset, mem.len, mem.ptr());
    gl.glBindBuffer(target, 0);
}

void GLESMemoryAllocator::buffer_bind_and_update_memory(GLID buffer, usize offset, Slice<const u8> mem, GLenum target, UpdateMemoryHint umh)
{
    gl.glBindBuffer(target, buffer);

    if (umh == UMHWriteOnly)
    {
        void* dest_ptr = gl.glMapBufferRange(target, offset, mem.len, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        Slice<u8> dest = Slice{ (u8*)dest_ptr, mem.len };
        mem::copy(dest, mem);
        gl.glUnmapBuffer(target);
    }
    else
    {
        gl.glBufferSubData(target, offset, mem.len, mem.ptr());
    }
}

GLID GLESMemoryAllocator::buffer_allocate_handle_and_fill(GLESBuffer& buffer, Slice<const u8> mem, GLenum target, GLenum usage)
{
    buffer.buffer = buffer_allocate_handle();
    buffer_fill_memory(buffer.buffer, mem, target, usage);
    buffer.size = mem.len;
    buffer.target = target;
    buffer.usage = usage;
    return buffer.buffer;
}

GLESMemoryAllocator::GLESTexture& GLESMemoryAllocator::texture_allocate()
{
    TextureID tex_id = data.textures.add(GLESTexture());
    GLESTexture& texture = data.textures.get(tex_id);
    texture.self = tex_id;
    return texture;
}

void GLESMemoryAllocator::texture_free(TextureID tex_id)
{
    GLESTexture& texture = texture_get(tex_id);
    gl.glDeleteTextures(1, &texture.texture);

    usize allocation_size = texture.size.width * texture.size.height* _get_format_size(texture.internal_format);
    data.allocated_bytes -= allocation_size;

    data.textures.remove(tex_id);
}

GLID GLESMemoryAllocator::texture_allocate_handle()
{
    GLID texture;
    gl.glGenTextures(1, &texture);
    return texture;
}

TextureID GLESMemoryAllocator::allocate_texture_from_info(const TextureCreateInfo& create_info)
{
    GLESMemoryAllocator::GLESTexture& tex = GLESMemoryAllocator::texture_allocate();
    tex.image = nullptr;
    tex.size = create_info.size;

    tex.texture = texture_allocate_handle_and_fill(tex, create_info);
    return tex.self;
}

GLID GLESMemoryAllocator::texture_allocate_handle_and_fill(GLESTexture& texture, const TextureCreateInfo& create_info)
{
    GLenum target = _get_texture_target(create_info.type);
    GLenum min_filter = _get_texture_filter(create_info.min_filter);
    GLenum mag_filter = _get_texture_filter(create_info.mag_filter);

    GLenum texture_format = _get_texture_format(create_info.format);
    GLenum internal_format = _get_texture_internal_format_for(texture_format);

    GLID texture_handle = texture_allocate_handle();
    texture_wrap(texture_handle, target, GL_CLAMP_TO_BORDER);
    texture_filter(texture_handle, target, min_filter, mag_filter);

    texture_allocate_memory(
        texture_handle,
        target,
        create_info.size,
        internal_format,
        texture_format,
        create_info.pixels
    );

    texture.size = create_info.size;
    texture.target = target;
    texture.internal_format = texture_format;
    texture.texture_format = texture_format;
    texture.min_filter = min_filter;
    texture.mag_filter = mag_filter;

    return texture_handle;
}

void GLESMemoryAllocator::texture_allocate_memory(GLID texture, GLenum target, const Vector2I& size,
    GLenum internal_format, GLenum input_format, Slice<u8> bytes)
{
    DebugAssert(
        bytes.len == 0 || size.width * size.height * _get_format_size(input_format) == bytes.len,
        "inconsistent buffer size"
    );

    gl.glBindTexture(target, texture);
    gl.glTexImage2D(
        target, 0, internal_format, size.width, size.height,
        0, input_format ? input_format : GL_RGBA, GL_UNSIGNED_BYTE, bytes.ptr()
    );
    gl.glBindTexture(target, 0);

    data.allocated_bytes += size.width * size.height * _get_format_size(internal_format);
}

GLESMemoryAllocator::GLESRenderTarget& GLESMemoryAllocator::render_target_allocate()
{
    RenderTargetID rt_id = data.render_targets.add(GLESRenderTarget());
    GLESRenderTarget& rt = render_target_get(rt_id);
    rt.self_id = rt_id;
    return rt;

}

void GLESMemoryAllocator::render_target_free(RenderTargetID rt_id)
{
    GLESRenderTarget& rt = render_target_get(rt_id);
    gl.glDeleteTextures(1, &rt.color_buffer);
    gl.glDeleteFramebuffers(1, &rt.framebuffer);

    usize allocation_size = rt.size.width * rt.size.height * _get_format_size(rt.format);
    data.allocated_bytes -= allocation_size;
}

GLID GLESMemoryAllocator::render_target_allocate_handle()
{
    GLID framebuffer;
    gl.glGenFramebuffers(1, &framebuffer);
    return framebuffer;
}

RenderTargetID GLESMemoryAllocator::allocate_render_target_from_info(const RenderTargetCreateInfo& create_info)
{
    GLESRenderTarget& rt = render_target_allocate();
    rt.size = create_info.size;
    rt.format = _get_texture_internal_format_for(_get_texture_format(create_info.format));

    rt.color_buffer = texture_allocate_handle();
    texture_filter(rt.color_buffer, GL_TEXTURE_2D, GL_NEAREST, GL_NEAREST);

    texture_allocate_memory(rt.color_buffer, GL_TEXTURE_2D, create_info.size, rt.format, GL_RGBA, {});

    rt.framebuffer = render_target_allocate_handle();
    render_target_bind_texture(rt.framebuffer, rt.color_buffer);

    return rt.self_id;
}

void GLESMemoryAllocator::render_target_bind_texture(GLID render_target, GLID texture)
{
    gl.glBindFramebuffer(GL_FRAMEBUFFER, render_target);
    gl.glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, texture, 0
    );
    gl.glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLESMemoryAllocator::GLESBuffer& GLESMemoryAllocator::buffer_get(ResourceID buffer_id)
{
    DebugAssert(buffer_id.id < data.buffers.count, "invalid buffer id");
    return data.buffers[buffer_id];
}

GLESMemoryAllocator::GLESTexture& GLESMemoryAllocator::texture_get(TextureID tex_id)
{
    return data.textures.get(tex_id);
}

void GLESMemoryAllocator::texture_set_image(TextureID tex_id, Image* image)
{
    texture_get(tex_id).image = image;
}

Vector2I GLESMemoryAllocator::texture_get_size(TextureID tex_id)
{
    return texture_get(tex_id).size;
}

GLID GLESMemoryAllocator::texture_get_handle(TextureID tex_id)
{
    return texture_get(tex_id).texture;
}

void GLESMemoryAllocator::texture_filter(GLID texture, GLenum target, GLenum min, GLenum mag)
{
    gl.glBindTexture(target, texture);
    gl.glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min);
    gl.glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mag);
    gl.glBindTexture(target, 0);
}

void GLESMemoryAllocator::texture_wrap(GLID texture, GLenum target, GLenum wrap)
{
    gl.glBindTexture(target, texture);
    gl.glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap);
    gl.glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap);
    gl.glBindTexture(target, 0);
}

GLESMemoryAllocator::GLESRenderTarget& GLESMemoryAllocator::render_target_get(RenderTargetID rt_id)
{
    return data.render_targets.get(rt_id);
}

Vector2I GLESMemoryAllocator::render_target_get_size(RenderTargetID rt_id)
{
    return render_target_get(rt_id).size;
}

void GLESMemoryAllocator::render_target_set_size(RenderTargetID rt_id, const Vector2I& new_size)
{
    // ignore backbuffer modifications
    if (rt_id == RenderTargetID(0))
        return;

    GLESRenderTarget& rt = render_target_get(rt_id);

    usize old_byte_size = rt.size.width * rt.size.height * _get_format_size(rt.format);
    data.allocated_bytes -= old_byte_size;
    texture_allocate_memory(rt.color_buffer, GL_TEXTURE_2D, new_size, rt.format, GL_RGBA, {});

        gl.glBindFramebuffer(GL_FRAMEBUFFER, rt.framebuffer);
        gl.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rt.color_buffer, 0);
        gl.glBindFramebuffer(GL_FRAMEBUFFER, 0);

    rt.size = new_size;
}

GLID GLESMemoryAllocator::render_target_get_handle(RenderTargetID rt_id)
{
    return render_target_get(rt_id).framebuffer;
}
