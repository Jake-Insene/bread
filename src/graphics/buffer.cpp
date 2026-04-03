#include "graphics/buffer.h"

#include "engine/engine.h"
#include "render/render_device.h"


namespace Graphics
{

void Buffer::init(const mem::Allocator& _allocator, const BufferInfo& info)
{
    allocator = _allocator;
    gpu_buffer = GPU::buffer_create(
        {
            .device = info.gpu_device,
            .usage = info.usage,
            .size = info.size,
            .memory_heap = info.heap.get()->memory_heap,
            .heap_offset = info.heap_offset,
        }
    );
    heap = info.heap;
}

void Buffer::destroy()
{
    GPU::buffer_destroy(gpu_buffer);
}

Slice<u8> Buffer::map(usize offset, usize len)
{
    return heap.get()->map(heap_offset + offset, len);
}

void Buffer::unmap(Slice<u8> memory)
{
    heap.get()->unmap(memory);
}

}
