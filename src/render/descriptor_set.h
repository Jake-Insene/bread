#pragma once
#include "gpu/gpu.h"
#include "render/buffer.h"
#include "render/sampler.h"


struct DescriptorSetInfo
{
    GPU::DeviceID device;
    GPU::DescriptorPoolID pool;
    GPU::DescriptorSetLayoutID set_layout;
};

struct DescriptorSet
{
    mem::Allocator allocator;

    GPU::DescriptorSetID descriptor_set;
    GPU::DescriptorSetLayoutID set_layout;

    void init(const mem::Allocator& _allocator, const DescriptorSetInfo& info);
    void destroy();

    void set_uniform_buffer(u32 binding, const Buffer& buffer, usize offset, usize range);
    void set_combined_texture_sampler(u32 binding, GPU::TextureID texture, GPU::TextureLayout layout, Sampler& sampler);
};