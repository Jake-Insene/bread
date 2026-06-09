#pragma once
#include "collections/array.h"
#include "gpu/gpu.h"
#include "graphics/descriptor_set.h"
#include "graphics/device_object.h"


namespace Graphics
{

struct DescriptorPool : DeviceObject
{
    GPU::DeviceID gpu_device;
    GPU::DescriptorPoolID gpu_descriptor_pool;

    // Allocated sets that require destruction.
    Array<DescriptorSet*> allocated_sets;

    void init(Mem::Allocator* _allocator, Device* _parent, const GPU::DescriptorPoolCreateInfo& info);
    void destroy();

    DescriptorSet* allocate(GPU::DescriptorSetLayoutID gpu_descriptor_set_layout);
    void free(DescriptorSet* descriptor_set);
};

}
