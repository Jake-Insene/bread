#pragma once
#include "collections/free_list.h"
#include "gpu/gpu.h"
#include "mem/allocator.h"
#include "graphics/descriptor_set.h"
#include "graphics/device_object.h"


namespace Graphics
{

using DescriptorSetRef = ID<u32, struct _DescriptorSetRefTag>;

struct DescriptorPool : DeviceObject
{
    GPU::DeviceID gpu_device;
    GPU::DescriptorPoolID gpu_descriptor_pool;

    FreeList<DescriptorSet, DescriptorSetRef> descriptor_sets;
    // Allocated sets that require destruction.
    Array<DescriptorSetRef> allocated_sets;

    void init(Mem::Allocator* _allocator, Device* _parent, const GPU::DescriptorPoolCreateInfo& info);
    void destroy();

    DescriptorSetRef allocate(GPU::DescriptorSetLayoutID set_layout);
    void free(DescriptorSetRef set_ref);

    DescriptorSet* set(DescriptorSetRef set_ref) { return &descriptor_sets.get(set_ref); }
};

}
