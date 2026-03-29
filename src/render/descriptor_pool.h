#pragma once
#include "collections/free_list.h"
#include "gpu/gpu.h"
#include "mem/allocator.h"
#include "render/descriptor_set.h"


struct DescriptorPool
{
    mem::Allocator allocator;

    GPU::DeviceID device;
    GPU::DescriptorPoolID descriptor_pool;

    Array<DescriptorSet> allocated_sets;

    static DescriptorPool create(u32 max_sets, Slice<const GPU::DescriptorPoolSize> sizes);

    void init(const mem::Allocator& _allocator, const GPU::DescriptorPoolCreateInfo& info);
    void destroy();

    DescriptorSet allocate(GPU::DescriptorSetLayoutID set_layout);
};
