#include "render/buffer.h"

#include "engine/engine.h"
#include "render/render_device.h"


Buffer Buffer::create(GPU::BufferUsage usage, usize size, Ptr<MemoryHeap> heap, usize heap_offset)
{
    RenderDevice* render_device = Engine::get_system_manager()->get_system<RenderDevice>();

    Buffer buffer = {};
    buffer.init(render_device->allocator,
        {
            .device = render_device->get_graphics_device(),
            .usage = usage,
            .size = size,
            .memory_heap = heap.get()->memory_heap,
            .heap_offset = heap_offset,
        },
        heap
    );

    return buffer;
}

void Buffer::init(const mem::Allocator& _allocator, const GPU::BufferCreateInfo& info, Ptr<MemoryHeap> _heap)
{
    allocator = _allocator;
    buffer = GPU::buffer_create(info);
    heap = _heap;
}

void Buffer::destroy()
{
    GPU::buffer_destroy(buffer);
}

Slice<u8> Buffer::map(usize offset, usize len)
{
    return heap.get()->map(heap_offset + offset, len);
}

void Buffer::unmap(Slice<u8> memory)
{
    heap.get()->unmap(memory);
}

