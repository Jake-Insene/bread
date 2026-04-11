#pragma once
#include "math/mat4.h"
#include "math/vec2.h"
#include "math/color.h"
#include "math/rect_2d.h"
#include "graphics/device.h"
#include "render_device/core/gpu_memory_allocator.h"
#include "renderer/framed_buffer.h"


struct GPUMemoryAllocator;

struct Renderer2DCreateInfo
{
    mem::Allocator allocator;
    Graphics::Device* graphics_device;
    GPUMemoryAllocator* gpu_memory_allocator;
};

struct Renderer2D
{
    static constexpr usize MaxInstancePerFramedBuffer = 128;

    struct BaseInstance
    {
        // attrib 0
        Vector2 xx;
        Vector2 yy;
        // attrib 1
        Vector2 zz;
        Color color;
        u32 material_index;
        // attrib 2
        Rect2D rect;
        // attrib 3
        Rect2D uv_rect;
    };

    mem::Allocator allocator;
    FramedBuffer buffers;

    void init(const Renderer2DCreateInfo& info);
    void destroy();
};
