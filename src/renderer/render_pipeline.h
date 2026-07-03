#pragma once
#include "gpu/gpu.h"
#include "graphics/shader.h"


struct RenderLayout
{
    Mem::Allocator* allocator;
    Slice<GPU::DescriptorSetLayoutID> set_layouts;
    GPU::PipelineLayoutID pipeline_layout;

    static RenderLayout create(Mem::Allocator* allocator, GPU::DeviceID device,
        const Slice<const GPU::ConstantBlock>& constant_blocks,
        const Slice<const GPU::DescriptorSetLayoutCreateInfo>& set_layout_cis);

    void destroy();
}; 

struct RenderPipeline
{
    Mem::Allocator* allocator;
    RenderLayout render_layout;
    GPU::PipelineID pipeline;

    static RenderPipeline create(Mem::Allocator* allocator, GPU::DeviceID device,
        RenderLayout render_layout, const Graphics::Shader* shader, GPU::VertexInput vertex_input,
        GPU::PrimitiveTopology topology, const GPU::RenderingInfo& rendering_info);

    void destroy();

    Slice<GPU::DescriptorSetLayoutID> get_set_layouts() const { return render_layout.set_layouts; }

    GPU::PipelineLayoutID get_pipeline_layout() const { return render_layout.pipeline_layout; }
    GPU::PipelineID get_pipeline() const { return pipeline; }
};
