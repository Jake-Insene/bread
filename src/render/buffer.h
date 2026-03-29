#pragma once
#include "collections/ptr.h"
#include "gpu/gpu.h"
#include "mem/allocator.h"
#include "render/memory_heap.h"


struct Buffer
{
    mem::Allocator allocator;

    GPU::BufferID buffer;
    Ptr<MemoryHeap> heap;
    usize heap_offset;
    
    static Buffer create(GPU::BufferUsage usage, usize size, Ptr<MemoryHeap> heap, usize heap_offset);

    void init(const mem::Allocator& _allocator, const GPU::BufferCreateInfo& info, Ptr<MemoryHeap> _heap);
    void destroy();

    Slice<u8> map(usize offset, usize len);
    void unmap(Slice<u8> memory);
};
