#include "graphics/pipeline_2d.h"


namespace Graphics
{

PipelineInfo Pipeline2D::make_default(const Pipeline2DInfo& info)
{
    return PipelineInfo
    {
        .bind_point = GPU::PipelineBindPoint::Graphics,
        .shader = info.shader,
        .vertex_input = info.vertex_input,
        .input_assembly = { .topology = info.primitive_topology },
        .rasterizer_state =
        {
            .depth_clamp_enable = false,
            .rasterizer_discard_enable = false,
            .polygon_mode = GPU::PolygonMode::Fill,
            .cull_mode = GPU::CullMode::Front,
            .front_face = GPU::FrontFace::ClockWise,
            .line_width = 1.F,
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
            .min_depth_bounds = 0.F,
            .max_depth_bounds = 1.F,
        },
        .pipeline_layout = info.pipeline_layout,
        .rendering_info = info.rendering_info,
    };
}

}
