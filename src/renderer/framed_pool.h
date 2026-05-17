#pragma once
#include "graphics/device.h"



struct FramedPoolCreateInfo
{
    mem::Allocator* allocator;
    Graphics::Device* graphics_device;

    Slice<GPU::DescriptorPoolSize> sizes;
    u32 max_sets;
    u32 frame_count;
};

struct FramedPool
{
    mem::Allocator* allocator;
    Graphics::Device* graphics_device;

    Array<Graphics::DescriptorPool*> pools;

    void init(const FramedPoolCreateInfo& info);
    void destroy();

    Graphics::DescriptorPool* get_pool(usize frame_index); 
};
