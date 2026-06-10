#include "renderer/render_pipeline.h"


RenderPipeline RenderPipeline::create(Mem::Allocator* allocator, GPU::DeviceID device,
    Graphics::Shader* shader, GPU::VertexInput vertex_input, GPU::PrimitiveTopology topology,
    const GPU::RenderingInfo& rendering_info)
{
    GPU::PipelineLayoutID pipeline_layout = GPU::pipeline_layout_create(
        device, GPU::PipelineLayoutCreateInfo::create({}, {})
    );

    return RenderPipeline
    {
        .allocator = allocator,
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
                .pipeline_layout = pipeline_layout,
                .rendering_info = rendering_info,
            }
        ),
        .pipeline_layout = pipeline_layout,
    };
}

void RenderPipeline::destroy()
{
    GPU::pipeline_destroy(pipeline);
    GPU::pipeline_layout_destroy(pipeline_layout);
}
