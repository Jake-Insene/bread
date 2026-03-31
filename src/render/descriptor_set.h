#pragma once
#include "collections/array.h"
#include "gpu/gpu.h"
#include "render/buffer.h"
#include "render/sampler.h"


struct DescriptorSetInfo
{
    GPU::DeviceID device;
    GPU::DescriptorPoolID pool;
    GPU::DescriptorSetLayoutID set_layout;
};

union WriteInfo
{
    struct
    {
        GPU::BufferID buffer;
        usize offset;
        usize range;
    } buffer;
    struct
    {
        GPU::TextureID texture;
        GPU::TextureLayout layout;
        GPU::SamplerID sampler;
    } texture;
};

struct DeferredWrite
{
    u32 binding;
    GPU::DescriptorType type;
    WriteInfo write;
};

struct DescriptorSet
{
    mem::Allocator allocator;

    Array<DeferredWrite> deferred_writes;
    usize deferred_buffers;
    usize deferred_textures;
    bool use_deferred;
    GPU::DescriptorSetID descriptor_set;
    GPU::DescriptorSetLayoutID set_layout;

    void init(const mem::Allocator& _allocator, const DescriptorSetInfo& info);
    void destroy();

    void set_deferred(bool _use_deferred) { use_deferred = _use_deferred; }

    void set_uniform_buffer(u32 binding, const Buffer& buffer, usize offset, usize range);
    void set_combined_texture_sampler(u32 binding, GPU::TextureID texture, GPU::TextureLayout layout, Sampler& sampler);

    void sync_writes();
};