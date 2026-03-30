#include "render/pipeline.h"

#include "engine/engine.h"
#include "render/render_device.h"


Pipeline Pipeline::create(const PipelineInfo &info)
{
    Pipeline pipe = {};

    RenderDevice* render_device = Engine::get_system_manager()->get_system<RenderDevice>();

    pipe.init(render_device->allocator, info);
    return pipe;
}

void Pipeline::init(const mem::Allocator _allocator, const PipelineInfo& info)
{
    allocator = _allocator;

    set_layouts = allocator.array<GPU::DescriptorSetLayoutID>(info.set_layout_infos.len);
    for(usize i = 0; i < info.set_layout_infos.len; i++)
    {
        GPU::DescriptorSetLayoutCreateInfo set_layout_info = info.set_layout_infos[i];
        set_layout_info.device = info.device;
        set_layouts[i] = GPU::descriptor_set_layout_create(set_layout_info);
    }

    GPU::ShaderStageInfo shader_stages[] =
    {
        { .stage = GPU::ShaderStage::Vertex, .code = info.shader.shader_code, .name = info.shader.shader_info.vertex_name, },
        { .stage = GPU::ShaderStage::Fragment, .code = info.shader.shader_code, .name = info.shader.shader_info.fragment_name, },
    };

    const GPU::PipelineCreateInfo pipeline_ci =
    {
        .device = info.device,
        .bind_point = info.bind_point,
        .shader_stages = shader_stages,
        .vertex_input = info.vertex_input,
        .input_assembly = info.input_assembly,
        .rasterizer_state = info.rasterizer_state,
        .multisample_state = info.multisample_state,
        .depth_stencil_state = info.depth_stencil_state,
        .pipeline_layout =
        {
            .constant_blocks = info.constant_blocks,
            .set_layouts = set_layouts,
        },
        .surface_format = info.surface_format,
    };

    pipeline = GPU::pipeline_create(pipeline_ci);

}

void Pipeline::destroy()
{
    for(GPU::DescriptorSetLayoutID set_layout : set_layouts)
    {
        GPU::descriptor_set_layout_destroy(set_layout);
    }
    allocator.free(mem::to_bytes(set_layouts));
    
    GPU::pipeline_destroy(pipeline);
}

GPU::DescriptorSetLayoutID Pipeline::get_set_layout(usize set_index)
{
    DebugAssert(set_index < set_layouts.len, "invalid set index");

    return set_layouts[set_index];
}

