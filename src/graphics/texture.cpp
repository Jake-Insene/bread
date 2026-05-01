#include "graphics/texture.h"


namespace Graphics
{

void Texture::init(const mem::Allocator& _allocator, Device* _parent, GPU::DeviceID gpu_device, const TextureInfo& info)
{
    DeviceObject::init(_allocator, _parent);
    gpu_texture = GPU::texture_create(
        {
            .device = gpu_device,
            .type = info.type,
            .format = info.format,
            .extent = info.extent,
            .mip_levels = info.mip_levels,
            .array_levels = info.array_levels,
            .sample_count = info.sample_count,
            .tiling = info.tiling,
            .usage = info.usage,
            .initial_layout = info.initial_layout,
            .subresource_range = info.subresource_range,
        }
    );
}

void Texture::destroy()
{
    DeviceObject::destroy();
}

GPU::MemoryRequirements Texture::get_requirements() const
{
    return GPU::texture_get_memory_requirements(gpu_texture);
}

void Texture::bind_memory(MemoryHeap* memory_heap, usize offset)
{
    GPU::texture_bind_memory_heap(gpu_texture,
        {
            .memory_heap = memory_heap->gpu_memory_heap,
            .heap_offset = offset,
        }
    );
}

}