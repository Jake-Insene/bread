#include "render/memory_heap.h"

#include "engine/engine.h"
#include "render/render_device.h"


MemoryHeap MemoryHeap::create(GPU::HeapUsage usage, usize size)
{
    RenderDevice* render_device = Engine::get_system_manager().get_system<RenderDevice>();

    MemoryHeap heap = {};
    heap.init(render_device->allocator,
        {
            .device = render_device->get_graphics_device(),
            .heap_usage = usage,
            .heap_size = size,
        }
    );

    return heap;
}

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
