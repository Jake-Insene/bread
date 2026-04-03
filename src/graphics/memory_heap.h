#pragma once
#include "gpu/gpu.h"
#include "mem/allocator.h"
#include "graphics/device_object.h"


namespace Graphics
{

struct MemoryHeap : DeviceObject
{
    GPU::MemoryHeapID memory_heap;

    void init(const mem::Allocator& _allocator, Device* _parent, const GPU::MemoryHeapCreateInfo& info);
    void destroy();

    Slice<u8> map(usize offset, usize len);
    void unmap(Slice<u8> memory);
};

}
