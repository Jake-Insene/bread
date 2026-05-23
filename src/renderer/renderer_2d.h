#pragma once
#include "math/mat4.h"
#include "math/transform_2d.h"
#include "math/vec2.h"
#include "math/color.h"
#include "math/rect_2d.h"
#include "graphics/device.h"
#include "graphics/sampler.h"
#include "graphics/pipeline_2d.h"
#include "render_device/core/gpu_memory_allocator.h"
#include "renderer/framed_buffer.h"
#include "renderer/framed_pool.h"
#include "renderer/renderer.h"
#include "renderer/renderer_batch_2d.h"


struct GPUMemoryAllocator;

struct Renderer2DCreateInfo : RendererCreateInfo
{
    u32 max_instances_per_primitive;
};

struct Renderer2D : Renderer
{    
    struct SceneUniform
    {
        Mat4 view;
        Mat4 projection;
        Mat4 view_projection;
    };

    RendererBatch2D batcher;
    Graphics::Sampler* sampler;

    void init(const Renderer2DCreateInfo& info);
    void destroy();

    virtual void render(const FrameInfo& frame_info) override;

    void draw_sprite(const Transform2D& transform, const Color& color, const Rect2D& rect, GPU::TextureID texture);
    void draw_quad(const Transform2D& transform, const Color& color, const Rect2D& rect);
    void draw_line(const Transform2D& transform, const Color& color, const Vector2& begin, const Vector2& end);
    void draw_circle(const Transform2D& transform, const Color& color, const Vector2& point, f32 radius);
};
