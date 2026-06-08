#pragma once
#include "gpu/gpu.h"
#include "mem/allocator.h"
#include "graphics/device_object.h"
#include "graphics/structures.h"


namespace Graphics
{

struct PipelineLayout : DeviceObject
{
    GPU::PipelineLayoutID gpu_pipeline_layout;
    Slice<GPU::DescriptorSetLayoutID> gpu_set_layouts;

    void init(Mem::Allocator* _allocator, Device* _parent, GPU::DeviceID gpu_device, const PipelineLayoutInfo& info);
    void destroy();

    GPU::DescriptorSetLayoutID get_layout(u32 set_index);
};

}
