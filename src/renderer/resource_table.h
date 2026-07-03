#pragma once
#include "collections/array.h"
#include "gpu/gpu.h"
#include "mem/stack_allocator.h"


struct ResourceTableCreateInfo
{
    Mem::Allocator* allocator;
    GPU::DeviceID device;
    Slice<const GPU::DescriptorSetLayoutID> set_layouts;
};

/**
* begin()/end() prepare the sets, but end doesn't deallocates sets
*/
struct ResourceTable
{
    struct BindedResource
    {
        GPU::DescriptorType descriptor_type;
        u32 set;
        u32 binding;

        union
        {
            GPU::DescriptorBufferInfo buffer;
            GPU::DescriptorTextureInfo texture;
        };
    };

    Mem::Allocator* allocator;
    GPU::DeviceID device;
    Slice<GPU::DescriptorSetLayoutID> set_layouts;
    Slice<GPU::DescriptorSetID> sets;

    Mem::StackAllocator tmp_allocator;
    Array<GPU::WriteDescriptorInfo> gpu_write_infos;

    static ResourceTable create(const ResourceTableCreateInfo& info);
    
    void destroy();

    void begin(GPU::DescriptorPoolID pool);
    void bind_combined_texture_sampler(u32 set, u32 binding, const GPU::DescriptorTextureInfo& texture);
    void bind_uniform_buffer(u32 set, u32 binding, const GPU::DescriptorBufferInfo& buffer);
    void end(GPU::DescriptorPoolID pool);
};
