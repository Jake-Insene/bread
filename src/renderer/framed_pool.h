#pragma once
#include "collections/array.h"
#include "gpu/gpu.h"



struct FramedPoolCreateInfo
{
    Mem::Allocator* allocator;
    GPU::DeviceID device;

    u32 frame_count;
    u32 max_sets;
    Slice<const GPU::DescriptorPoolSize> sizes;
};

struct FramedPool
{
    struct InternalData
    {
        Mem::Allocator* allocator;
        GPU::DeviceID device;

        Array<GPU::DescriptorPoolID> pools;
    } data;

    void init(const FramedPoolCreateInfo& info);
    void destroy();

    void reset_pool(usize frame_index);
    GPU::DescriptorPoolID get_pool(usize frame_index); 
};
