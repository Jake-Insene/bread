#pragma once
#include "gpu/gpu.h"
#include "mem/allocator.h"
#include "graphics/device_object.h"


namespace Graphics
{

struct MemoryHeap : DeviceObject
{
    GPU::MemoryHeapID gpu_memory_heap;
    usize heap_size;
    u32 map_count = 0;
    Slice<u8> mapped_memory;

    void init(Mem::Allocator* _allocator, Device* _parent, const GPU::MemoryHeapCreateInfo& info);
    void destroy();

    Slice<u8> map(usize offset, usize len);
    void unmap(const Slice<u8>& memory);
};

}
