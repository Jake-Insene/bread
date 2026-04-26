#pragma once
#include "collections/ptr.h"
#include "gpu/gpu.h"
#include "mem/allocator.h"
#include "graphics/device_object.h"
#include "graphics/memory_heap.h"


namespace Graphics
{

struct BufferInfo
{
    GPU::DeviceID gpu_device;
    GPU::BufferUsage usage;
    usize size;
    MemoryHeap* heap;
    usize heap_offset;
};

struct Buffer : DeviceObject
{
    GPU::BufferID gpu_buffer;
    MemoryHeap* heap;
    usize heap_offset;
    
    void init(const mem::Allocator& _allocator, Device* _parent, const BufferInfo& info);
    void destroy();

    Slice<u8> map(usize offset, usize len);
    void unmap(const Slice<u8>& memory);
};

}
