#pragma once
#include "gpu/gpu.h"
#include "graphics/device_object.h"
#include "graphics/shader.h"



struct RenderPipeline
{
    Mem::Allocator* allocator;
    Graphics::Pipeline* pipeline;
    Graphics::PipelineLayout* pipeline_layout;

    static RenderPipeline create(Mem::Allocator* allocator, Graphics::Device* graphics_device,
        Graphics::Shader* shader, GPU::VertexInput vertex_input, GPU::PrimitiveTopology topology,
        const GPU::RenderingInfo& rendering_info);

    void destroy();
};
