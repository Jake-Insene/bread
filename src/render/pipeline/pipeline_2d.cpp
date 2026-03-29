#include "render/pipeline/pipeline_2d.h"

#include "engine/engine.h"
#include "render/render_device.h"


Pipeline2D Pipeline2D::make_default(const Pipeline2DInfo& info)
{
    RenderDevice* render_device = Engine::get_system_manager()->get_system<RenderDevice>();

    Pipeline2D pipe = {};
    pipe.init(
        Engine::get_system_manager()->get_system<RenderDevice>()->allocator,
        {
            .device = render_device->get_graphics_device(),
            .bind_point = GPU::PipelineBindPoint::Graphics,
            .shader = info.shader,
            .vertex_input = info.vertex_input,
            .input_assembly = { .topology = GPU::PrimitiveTopology::TriangleList },
            .rasterizer_state =
            {
                .depth_clamp_enable = false,
                .rasterizer_discard_enable = false,
                .polygon_mode = GPU::PolygonMode::Fill,
                .cull_mode = GPU::CullMode::Front,
                .front_face = GPU::FrontFace::ClockWise,
                .line_width = 1.f,
            },
            .multisample_state =
            {
                .sample_count = GPU::SampleCount::Sample1,
                .min_sample_shading = 0,
                .sample_shading_enable = false,
                .alpha_to_coverage_enable = false,
                .alpha_one_enable = false,
            },
            .depth_stencil_state =
            {
                .depth_test_enable = false,
                .depth_write_enable = false,
                .depth_bounds_test_enable = false,
                .stencil_test_enable = false,
                .min_depth_bounds = 0.f,
                .max_depth_bounds = 1.f,
            },
            .constant_blocks = info.constant_blocks,
            .set_layout_infos = info.set_layout_infos,
            .surface_format = info.surface_format,
        }
    );
 
    return pipe;
}