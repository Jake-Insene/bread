#include "graphics/buffer.h"


namespace Graphics
{

void Buffer::init(const mem::Allocator& _allocator, Device* _parent, const BufferInfo& info)
{
    DeviceObject::init(_allocator, _parent);
    gpu_buffer = GPU::buffer_create(
        {
            .device = info.gpu_device,
            .usage = info.usage,
            .size = info.size,
            .memory_heap = info.heap->memory_heap,
            .heap_offset = info.heap_offset,
        }
    );
    heap = info.heap;
}

void Buffer::destroy()
{
    GPU::buffer_destroy(gpu_buffer);
    DeviceObject::destroy();
}

Slice<u8> Buffer::map(usize offset, usize len)
{
    return heap->map(heap_offset + offset, len);
}

void Buffer::unmap(const Slice<u8>& memory)
{
    heap->unmap(memory);
}

}
