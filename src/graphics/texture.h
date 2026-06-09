#pragma once
#include "gpu/gpu.h"
#include "graphics/device_object.h"
#include "graphics/structures.h"


namespace Graphics
{

struct Texture : DeviceObject
{
    GPU::TextureID gpu_texture;
	MemoryHeap* heap;
    usize heap_offset;
    GPU::MemoryRequirements gpu_memory_requirements;
    
    void init(Mem::Allocator* _allocator, Device* _parent, const TextureInfo& info);
    void destroy();

	GPU::MemoryRequirements get_requirements() const;
    void bind_memory(MemoryHeap* memory_heap, usize offset);
};

}
