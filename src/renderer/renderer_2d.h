#pragma once
#include "math/mat4.h"
#include "math/vec2.h"
#include "math/color.h"
#include "math/rect_2d.h"
#include "graphics/device.h"
#include "graphics/pipeline_2d.h"
#include "render_device/core/gpu_memory_allocator.h"
#include "renderer/framed_buffer.h"
#include "renderer/framed_pool.h"
#include "renderer/renderer.h"


struct GPUMemoryAllocator;

struct Renderer2DCreateInfo : RendererCreateInfo
{
};

struct Renderer2D : Renderer
{    
    struct SceneUniform
    {
        Mat4 view;
        Mat4 projection;
        Mat4 view_projection;
    };

    struct alignas(Vector4) BaseInstance
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
    static_assert(sizeof(BaseInstance) / sizeof(Vector4) <= 5, "BaseInstance should use more than 5 attributes");
    
    static constexpr usize InstanceSize = sizeof(Vector4) * GPU::MaxVertexInputAttributes;
    static constexpr usize AttributesPerInstance = InstanceSize / sizeof(Vector4);

    static constexpr usize MaxInstancePerFramedBuffer = 128;
    static constexpr usize InstanceBufferSize = MaxInstancePerFramedBuffer * sizeof(InstanceSize);

    Graphics::Pipeline* pipeline;

    FramedDeviceBuffer buffers;
    FramedMappedBuffer uniform_buffers;
    FramedPool uniform_pool;

    void init(const Renderer2DCreateInfo& info);
    void destroy();

    void build_frame(const FrameInfo& frame_info);
};
