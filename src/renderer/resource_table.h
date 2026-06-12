#pragma once
#include "gpu/gpu.h"


struct ResourceTableCreateInfo
{
    Mem::Allocator* allocator;
    GPU::DeviceID device;
    u32 max_sets;
    Slice<const GPU::DescriptorPoolSize> sizes;
};

struct ResourceTable
{
    Mem::Allocator* allocator;

    GPU::DeviceID device;
    GPU::DescriptorPoolID descriptor_pool;

    static ResourceTable create(const ResourceTableCreateInfo& info);
    
    void destroy();

    void reset();
    GPU::DescriptorSetID allocate(GPU::DescriptorSetLayoutID set_layout);
};
