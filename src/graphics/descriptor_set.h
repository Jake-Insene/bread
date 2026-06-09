#pragma once
#include "gpu/gpu.h"
#include "graphics/device_object.h"
#include "graphics/structures.h"


namespace Graphics
{

struct DescriptorSet : DeviceObject
{
    GPU::DescriptorSetID gpu_descriptor_set;
    GPU::DescriptorPoolID gpu_descriptor_pool;
    GPU::DescriptorSetLayoutID gpu_descriptor_set_layout;

    void init(Mem::Allocator* _allocator, Device* _parent, const DescriptorSetInfo& info);
    void destroy();
};

}
