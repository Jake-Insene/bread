#pragma once
#include "gpu/gpu.h"
#include "mem/allocator.h"


namespace Graphics
{

struct MemoryHeap
{
    mem::Allocator allocator;

    GPU::MemoryHeapID memory_heap;

    void init(const mem::Allocator& allocator, const GPU::MemoryHeapCreateInfo& info);
    void destroy();

    Slice<u8> map(usize offset, usize len);
    void unmap(Slice<u8> memory);
};

}
