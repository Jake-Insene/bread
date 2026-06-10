#pragma once
#include "renderer/framed_buffer.h"
#include "renderer/scene_renderer.h"
#include "renderer/renderer.h"


struct GPUMemoryAllocator;

struct Renderer2D : Renderer
{
    // 4Kb
    static constexpr usize MaxSceneUniformSize = 4096;

    GPU::SamplerID sampler;
    FramedMappedBuffer scene_uniform_buffer;
    GPU::DescriptorSetLayoutID global_set_layout;
    GPU::PipelineLayoutID global_scene_layout;
    GPU::DescriptorPoolID global_scene_pool;
    Slice<GPU::DescriptorSetID> global_scene_set;

    void init(const RendererCreateInfo& info);
    void destroy();

    SceneRenderer::SceneUniform* get_scene_uniform(const FrameInfo& frame_info);
    GPU::DescriptorSetID get_global_set(const FrameInfo& frame_info);
};
