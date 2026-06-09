#pragma once
#include "gpu/gpu.h"
#include "graphics/device_object.h"


struct ResourceTableLineLayout
{
    Slice<const GPU::DescriptorBinding> bindings;
};

struct ResourceTableCreateInfo
{
    Mem::Allocator* allocator;
    Graphics::Device* graphics_device;
    u32 max_frames_in_flight;
    Slice<ResourceTableLineLayout> lines;
};

struct ResourceTable
{
    Mem::Allocator* allocator;

    Graphics::Device* device;
    Graphics::DescriptorPool* pool;

    void init(const ResourceTableCreateInfo& info);
    void destroy();
};
