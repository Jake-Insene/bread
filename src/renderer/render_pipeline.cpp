#include "renderer/render_pipeline.h"

#include "graphics/device.h"
#include "graphics/pipeline.h"
#include "graphics/pipeline_layout.h"


RenderPipeline RenderPipeline::create(Mem::Allocator* allocator, Graphics::Device* graphics_device,
    Graphics::Shader* shader, GPU::VertexInput vertex_input, GPU::PrimitiveTopology topology,
    const GPU::RenderingInfo& rendering_info)
{
    Graphics::PipelineLayout* pipeline_layout = graphics_device->create_pipeline_layout(
        {
        }
    );

    return RenderPipeline
    {
        .allocator = allocator,
        .pipeline = graphics_device->create_pipeline(
            {
                .bind_point = GPU::PipelineBindPoint::Graphics,
                .shader = shader,
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
    pipeline->destroy();
    pipeline_layout->destroy();
}
