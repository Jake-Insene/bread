#include "graphics/pipeline_layout.h"


namespace Graphics
{


void Graphics::PipelineLayout::init(mem::Allocator* _allocator, Device* _parent, GPU::DeviceID gpu_device, const PipelineLayoutInfo& info)
{
    DeviceObject::init(_allocator, _parent);
    gpu_set_layouts = allocator->array<GPU::DescriptorSetLayoutID>(info.set_layout_infos.len);
    for(usize i = 0; i < info.set_layout_infos.len; i++)
    {
        gpu_set_layouts[i] = GPU::descriptor_set_layout_create(
            {
                .device = gpu_device,
                .bindings = info.set_layout_infos[i].bindings,
            }
        );
    }

    gpu_pipeline_layout = GPU::pipeline_layout_create(
        {
            .device = gpu_device,
            .constant_blocks = info.constant_blocks,
            .set_layouts = gpu_set_layouts,
        }
    );
}

void PipelineLayout::destroy()
{
    for(GPU::DescriptorSetLayoutID gpu_set_layout : gpu_set_layouts)
    {
        GPU::descriptor_set_layout_destroy(gpu_set_layout);
    }
    allocator->free(mem::to_bytes(gpu_set_layouts));

    GPU::pipeline_layout_destroy(gpu_pipeline_layout);
    DeviceObject::destroy();
}

GPU::DescriptorSetLayoutID PipelineLayout::get_layout(u32 set_index)
{
    return gpu_set_layouts[set_index];
}

}
