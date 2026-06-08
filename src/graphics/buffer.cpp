#include "graphics/buffer.h"


namespace Graphics
{

void Buffer::init(Mem::Allocator* _allocator, Device* _parent, const BufferInfo& info)
{
    DeviceObject::init(_allocator, _parent);
    gpu_buffer = GPU::buffer_create(
        {
            .device = info.gpu_device,
            .usage = info.usage,
            .size = info.size,
        }
    );
    heap = nullptr;
    heap_offset = 0;
    gpu_memory_requirements = GPU::buffer_get_memory_requirements(gpu_buffer);
}

void Buffer::destroy()
{
    GPU::buffer_destroy(gpu_buffer);
    DeviceObject::destroy();
}

GPU::MemoryRequirements Buffer::get_requirements() const
{
    return gpu_memory_requirements;
}

void Buffer::bind_memory(MemoryHeap* memory_heap, usize offset)
{
    heap = memory_heap;
    heap_offset = offset;

    GPU::buffer_bind_memory_heap(gpu_buffer,
        {
            .memory_heap = memory_heap->gpu_memory_heap,
            .heap_offset = offset,
        }
    );
}

Slice<u8> Buffer::map(usize offset, usize len) const
{
    return heap->map(heap_offset + offset, len);
}

void Buffer::unmap(const Slice<u8>& memory) const
{
    heap->unmap(memory);
}

}
