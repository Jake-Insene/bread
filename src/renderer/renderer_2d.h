#pragma once
#include "math/mat4.h"
#include "math/transform_2d.h"
#include "math/vec2.h"
#include "math/color.h"
#include "math/rect_2d.h"
#include "graphics/device.h"
#include "graphics/sampler.h"
#include "graphics/pipeline_2d.h"
#include "renderer/framed_buffer.h"
#include "renderer/framed_pool.h"
#include "renderer/scene_renderer.h"
#include "renderer/renderer.h"
#include "renderer/renderer_batch_2d.h"


struct GPUMemoryAllocator;

struct Renderer2DCreateInfo : RendererCreateInfo
{
    u32 max_instances_per_primitive;
};

struct Renderer2D : Renderer
{
    // 4Kb
    static constexpr usize MaxSceneUniformSize = 4096;

    RendererBatch2D batcher;
    Graphics::Sampler* sampler;
    FramedMappedBuffer scene_uniform_buffer;
    Graphics::PipelineLayout* global_scene_layout;
    Graphics::DescriptorPool* global_scene_pool;
    Slice<Graphics::DescriptorSetRef> global_scene_set;

    void init(const Renderer2DCreateInfo& info);
    void destroy();

    SceneRenderer::SceneUniform* get_scene_uniform(const FrameInfo& frame_info);

    virtual void render(const FrameInfo& frame_info) override;

    void draw_sprite(const Transform2D& transform, const Color& color, const Rect2D& rect, GPU::TextureViewID texture_view);
    void draw_quad(const Transform2D& transform, const Color& color, const Rect2D& rect);
    void draw_line(const Transform2D& transform, const Color& color, const Vector2& begin, const Vector2& end);
    void draw_circle(const Transform2D& transform, const Color& color, const Vector2& point, f32 radius);

    virtual void update_scene_uniform(SceneRenderer::SceneUniform* scene_uniform) = 0;
};
