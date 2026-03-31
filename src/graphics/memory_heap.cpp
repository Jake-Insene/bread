#include "graphics/memory_heap.h"


namespace Graphics
{

void MemoryHeap::init(const mem::Allocator& _allocator, const GPU::MemoryHeapCreateInfo& info)
{
    allocator = _allocator;
    memory_heap = GPU::memory_heap_create(info);
}

void MemoryHeap::destroy()
{
    GPU::memory_heap_destroy(memory_heap);
}

Slice<u8> MemoryHeap::map(usize offset, usize len)
{
    return GPU::memory_heap_map(memory_heap, offset, len);
}

void MemoryHeap::unmap(Slice<u8> memory)
{
    GPU::memory_heap_unmap(memory_heap, memory);
}

}
