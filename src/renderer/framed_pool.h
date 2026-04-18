#pragma once
#include "graphics/device.h"



struct FramedPoolCreateInfo
{
    mem::Allocator allocator;
    Graphics::Device* graphics_device;

    Slice<GPU::DescriptorPoolSize> sizes;
    u32 max_sets;
    u32 frame_count;

    GPU::DescriptorSetLayoutID gpu_set_layout;
};

struct FramedPool
{
    mem::Allocator allocator;
    Graphics::Device* graphics_device;

    Graphics::DescriptorPool* descriptor_pool;
    Array<Graphics::DescriptorSetRef> framed_sets;

    void init(const FramedPoolCreateInfo& info);
    void destroy();

    Graphics::DescriptorSet* get_set(usize frame_index); 
};
