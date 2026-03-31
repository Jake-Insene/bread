#pragma once
#include "collections/ptr.h"
#include "gpu/gpu.h"
#include "mem/allocator.h"
#include "graphics/memory_heap.h"


namespace Graphics
{

struct BufferInfo
{
    GPU::DeviceID device;
    GPU::BufferUsage usage;
    usize size;
    Ptr<MemoryHeap> heap;
    usize heap_offset;
};

struct Buffer
{
    mem::Allocator allocator;

    GPU::BufferID gpu_buffer;
    Ptr<MemoryHeap> heap;
    usize heap_offset;
    
    void init(const mem::Allocator& _allocator, const BufferInfo& info);
    void destroy();

    Slice<u8> map(usize offset, usize len);
    void unmap(Slice<u8> memory);
};

}
