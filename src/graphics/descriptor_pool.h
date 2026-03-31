#pragma once
#include "collections/free_list.h"
#include "gpu/gpu.h"
#include "mem/allocator.h"
#include "graphics/descriptor_set.h"


namespace Graphics
{

using DescriptorSetRef = ID<u32, struct _DescriptorSetRefTag>;

struct DescriptorPool
{
    mem::Allocator allocator;

    GPU::DeviceID device;
    GPU::DescriptorPoolID descriptor_pool;

    FreeList<DescriptorSet, DescriptorSetRef> descriptor_sets;
    // Free sets that it can be reused.
    Array<DescriptorSetRef> available_sets;
    // Allocated sets that require destruction.
    Array<DescriptorSetRef> allocated_sets;

    void init(const mem::Allocator& _allocator, const GPU::DescriptorPoolCreateInfo& info);
    void destroy();

    DescriptorSetRef allocate(GPU::DescriptorSetLayoutID set_layout);
    void free(DescriptorSetRef set_ref);

    DescriptorSet& set(DescriptorSetRef set_ref) { return descriptor_sets.get(set_ref); }
};

}
