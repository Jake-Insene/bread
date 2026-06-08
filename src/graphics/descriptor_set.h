#pragma once
#include "collections/array.h"
#include "gpu/gpu.h"
#include "graphics/buffer.h"
#include "graphics/device_object.h"
#include "graphics/structures.h"


namespace Graphics
{

struct DescriptorSet : DeviceObject
{
    Array<DescriptorDeferredWrite> deferred_writes;
    usize deferred_buffers;
    usize deferred_textures;
    bool use_deferred;
    GPU::DescriptorSetID gpu_descriptor_set;
    GPU::DescriptorSetLayoutID gpu_descriptor_set_layout;

    void init(Mem::Allocator* _allocator, Device* _parent, const DescriptorSetInfo& info);
    void destroy();

    void set_deferred(bool _use_deferred) { use_deferred = _use_deferred; }

    void set_uniform_buffer(u32 binding, const Buffer* buffer, usize offset, usize range);
    void set_combined_texture_sampler(u32 binding, GPU::TextureViewID texture_view, GPU::TextureLayout layout, Sampler* sampler);
    void set_combined_texture_sampler_array(u32 binding, Slice<GPU::TextureViewID> texture_views, GPU::TextureLayout layout, Slice<Graphics::Sampler*> samplers);

    void sync_writes();
};

}
