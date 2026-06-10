#pragma once
#include "collections/array.h"
#include "gpu/gpu.h"



struct FramedPoolCreateInfo
{
    Mem::Allocator* allocator;
    GPU::DeviceID device;

    Slice<GPU::DescriptorPoolSize> sizes;
    u32 max_sets;
    u32 frame_count;
};

struct FramedPool
{
    Mem::Allocator* allocator;
    GPU::DeviceID device;

    Array<GPU::DescriptorPoolID> pools;

    void init(const FramedPoolCreateInfo& info);
    void destroy();

    GPU::DescriptorPoolID get_pool(usize frame_index); 
};
