#include "renderer/render_pipeline.h"


RenderLayout RenderLayout::create(Mem::Allocator* allocator, GPU::DeviceID device,
    const Slice<const GPU::ConstantBlock>& constant_blocks,
        const Slice<const GPU::DescriptorSetLayoutCreateInfo>& set_layout_cis)
{
    Slice set_layouts = allocator->array<GPU::DescriptorSetLayoutID>(set_layout_cis.len);
    for(usize i = 0; i < set_layouts.len; i++)
    {
        set_layouts[i] = GPU::descriptor_set_layout_create(device, set_layout_cis[i]);
    }

    return RenderLayout
    {
        .allocator = allocator,
        .set_layouts = set_layouts,
        .pipeline_layout = GPU::pipeline_layout_create(device,
            GPU::PipelineLayoutCreateInfo::create(constant_blocks, set_layouts)),
    };   
}

void RenderLayout::destroy()
{
    for(GPU::DescriptorSetLayoutID set_layout : set_layouts)
    {
        GPU::descriptor_set_layout_destroy(set_layout);
    }
    allocator->free(Mem::to_bytes(set_layouts));
    
    GPU::pipeline_layout_destroy(pipeline_layout);
}

RenderPipeline RenderPipeline::create(Mem::Allocator* allocator, GPU::DeviceID device,
    RenderLayout render_layout, Graphics::Shader* shader, GPU::VertexInput vertex_input,
    GPU::PrimitiveTopology topology, const GPU::RenderingInfo& rendering_info)
{
    return RenderPipeline
    {
        .allocator = allocator,
        .render_layout = render_layout,
        .pipeline = GPU::pipeline_create(
            device,
            {
                .bind_point = GPU::PipelineBindPoint::Graphics,
                .shader_stages = shader->get_stages(),
                .vertex_input = vertex_input,
                .input_assembly = { .topology = topology },
                .rasterizer_state = GPU::RasterizerState::state(GPU::PolygonMode::Fill,
                    GPU::CullMode::Front, GPU::FrontFace::ClockWise),
                .multisample_state = GPU::MultisampleState::disable(),
                .depth_stencil_state = GPU::DepthStencilState::depth_stencil_disable(),
                .pipeline_layout = render_layout.pipeline_layout,
                .rendering_info = rendering_info,
            }
        ),
    };
}

void RenderPipeline::destroy()
{
    render_layout.destroy();
    GPU::pipeline_destroy(pipeline);
}
