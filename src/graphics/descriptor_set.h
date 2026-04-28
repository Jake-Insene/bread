#pragma once
#include "collections/array.h"
#include "gpu/gpu.h"
#include "graphics/buffer.h"
#include "graphics/sampler.h"


namespace Graphics
{

struct DescriptorSetInfo
{
    GPU::DeviceID gpu_device;
    GPU::DescriptorPoolID gpu_pool;
    GPU::DescriptorSetLayoutID gpu_set_layout;
};

union WriteInfo
{
    GPU::DescriptorBufferInfo buffer;
    GPU::DescriptorTextureInfo texture;
};

struct WriteArrayInfo
{
    Slice<GPU::DescriptorBufferInfo> buffers;
    Slice<GPU::DescriptorTextureInfo> textures;
};

struct DeferredWrite
{
    u32 binding;
    GPU::DescriptorType type;
    WriteInfo write;
    WriteArrayInfo write_array;
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

    void set_uniform_buffer(u32 binding, const Buffer* buffer, usize offset, usize range);
    void set_combined_texture_sampler(u32 binding, GPU::TextureID texture, GPU::TextureLayout layout, Sampler* sampler);
    void set_combined_texture_sampler_array(u32 binding, Slice<GPU::TextureID> textures, GPU::TextureLayout layout, Slice<Graphics::Sampler*> samplers);

    void sync_writes();
};

}
