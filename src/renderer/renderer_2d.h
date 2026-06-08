#pragma once
#include "graphics/sampler.h"
#include "renderer/framed_buffer.h"
#include "renderer/scene_renderer.h"
#include "renderer/renderer.h"


struct GPUMemoryAllocator;

struct Renderer2D : Renderer
{
    // 4Kb
    static constexpr usize MaxSceneUniformSize = 4096;

    Graphics::Sampler* sampler;
    FramedMappedBuffer scene_uniform_buffer;
    Graphics::PipelineLayout* global_scene_layout;
    Graphics::DescriptorPool* global_scene_pool;
    Slice<Graphics::DescriptorSet*> global_scene_set;

    void init(const RendererCreateInfo& info);
    void destroy();

    SceneRenderer::SceneUniform* get_scene_uniform(const FrameInfo& frame_info);
    Graphics::DescriptorSet* get_global_set(const FrameInfo& frame_info);
};
