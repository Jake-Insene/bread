#include "graphics/memory_heap.h"


namespace Graphics
{

void MemoryHeap::init(const mem::Allocator& _allocator, Device* _parent, const GPU::MemoryHeapCreateInfo& info)
{
    DeviceObject::init(_allocator, _parent);
    heap_size = info.heap_size;
    gpu_memory_heap = GPU::memory_heap_create(info);
}

void MemoryHeap::destroy()
{
    GPU::memory_heap_destroy(gpu_memory_heap);
    DeviceObject::destroy();
}

Slice<u8> MemoryHeap::map(usize offset, usize len)
{
    if (map_count == 0)
    {
        mapped_memory = GPU::memory_heap_map(gpu_memory_heap, 0, heap_size);
    }
    map_count++;
    return Slice<u8>(mapped_memory.items + offset, len);
}

void MemoryHeap::unmap(const Slice<u8>&)
{
    map_count--;
    if (map_count == 0)
    {
        GPU::memory_heap_unmap(gpu_memory_heap, mapped_memory);
    }
}

}
