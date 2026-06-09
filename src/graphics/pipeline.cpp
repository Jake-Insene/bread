#include "graphics/pipeline.h"

#include "graphics/pipeline_layout.h"



namespace Graphics
{

void Pipeline::init(Mem::Allocator* _allocator, Device* _parent, GPU::DeviceID gpu_device, const PipelineInfo& info)
{
    DeviceObject::init(_allocator, _parent);

    GPU::ShaderStageInfo shader_stages[] =
    {
        { .stage = GPU::ShaderStage::Vertex, .code = info.shader->shader_code, .name = info.shader->shader_info.vertex_name, },
        { .stage = GPU::ShaderStage::Fragment, .code = info.shader->shader_code, .name = info.shader->shader_info.fragment_name, },
    };

    const GPU::PipelineCreateInfo pipeline_ci =
    {
        .device = gpu_device,
        .bind_point = info.bind_point,
        .shader_stages = shader_stages,
        .vertex_input = info.vertex_input,
        .input_assembly = info.input_assembly,
        .rasterizer_state = info.rasterizer_state,
        .multisample_state = info.multisample_state,
        .depth_stencil_state = info.depth_stencil_state,
        .pipeline_layout = info.pipeline_layout->gpu_pipeline_layout,
        .rendering_info = info.rendering_info,
    };

    gpu_pipeline = GPU::pipeline_create(pipeline_ci);
}

void Pipeline::destroy()
{    
    GPU::pipeline_destroy(gpu_pipeline);
    DeviceObject::destroy();
}


}
