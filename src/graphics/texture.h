#pragma once
#include "collections/ptr.h"
#include "gpu/gpu.h"
#include "mem/allocator.h"
#include "graphics/device_object.h"
#include "graphics/memory_heap.h"


namespace Graphics
{

struct TextureInfo
{
    GPU::TextureType type;
	GPU::TextureFormat format;
	Vector3U extent;
	u32 mip_levels;
	u32 array_levels;
	GPU::SampleCount sample_count;
	GPU::TextureTiling tiling;
	GPU::TextureUsage usage;
	GPU::TextureLayout initial_layout;
    GPU::TextureSubresourceRanges subresource_range;
};

struct Texture : DeviceObject
{
    GPU::TextureID gpu_texture;
    
    void init(Mem::Allocator* _allocator, Device* _parent, GPU::DeviceID gpu_device, const TextureInfo& info);
    void destroy();

	GPU::MemoryRequirements get_requirements() const;
    void bind_memory(MemoryHeap* memory_heap, usize offset);
};

}
