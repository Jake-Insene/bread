#pragma once
#include "gpu/gpu.h"
#include "graphics/shader.h"



struct RenderPipeline
{
    Mem::Allocator* allocator;
    GPU::PipelineID pipeline;
    GPU::PipelineLayoutID pipeline_layout;

    static RenderPipeline create(Mem::Allocator* allocator, GPU::DeviceID device,
        Graphics::Shader* shader, GPU::VertexInput vertex_input, GPU::PrimitiveTopology topology,
        const GPU::RenderingInfo& rendering_info);

    void destroy();
};
