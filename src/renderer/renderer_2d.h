#pragma once
#include "math/mat4.h"
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

    void commit_sprite(const RendererBatch2D::SpriteInstance& sprite, GPU::TextureID texture);
    void commit_quad(const RendererBatch2D::QuadInstance& quad);
    void commit_line(const RendererBatch2D::LineInstance& line);
    void commit_circle(const RendererBatch2D::CircleInstance& circle);
};
