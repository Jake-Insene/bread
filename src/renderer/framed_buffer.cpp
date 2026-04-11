#include "renderer/framed_buffer.h"

#include "graphics/buffer.h"


void FramedBuffer::init(const FramedBufferCreateInfo& info)
{
    allocator = info.allocator;
    graphics_device = info.graphics_device;
    gpu_memory_allocator = info.gpu_memory_allocator;

    buffer_size = mem::align_up(info.buffer_size, GPU::MinHeapResourceAlignment);

    buffer_allocation = gpu_memory_allocator->allocate(GPUMemoryAllocator::AllocationTag::Buffer, info.frame_count * buffer_size);
    staging_buffer_allocation = gpu_memory_allocator->allocate(GPUMemoryAllocator::AllocationTag::Staging, info.frame_count * buffer_size);
   
    buffers = Array<Graphics::Buffer*>::with_size(allocator, info.frame_count);
    buffers.resize(info.frame_count);
    staging_buffers = Array<Graphics::Buffer*>::with_size(allocator, info.frame_count);
    staging_buffers.resize(info.frame_count);
    for(usize i = 0; i < info.frame_count; i++)
    {
        buffers.get(i) = graphics_device->create_buffer(
            info.usage | GPU::BufferUsage::TransferDestination, buffer_size,
            gpu_memory_allocator->allocation_get_heap(buffer_allocation),
            gpu_memory_allocator->allocation_get_offset(buffer_allocation) + (i * buffer_size)
        ).get();
        staging_buffers.get(i) = graphics_device->create_buffer(
            info.usage | GPU::BufferUsage::TransferSource, buffer_size,
            gpu_memory_allocator->allocation_get_heap(buffer_allocation),
            gpu_memory_allocator->allocation_get_offset(buffer_allocation) + (i * buffer_size)
        ).get();
    }
}

void FramedBuffer::destroy()
{
    (void)buffers.iter().for_each([](Graphics::Buffer* buffer){ buffer->destroy(); });
    (void)staging_buffers.iter().for_each([](Graphics::Buffer* buffer){ buffer->destroy(); });
    buffers.destroy();
    staging_buffers.destroy();

    gpu_memory_allocator->free(buffer_allocation);
    gpu_memory_allocator->free(staging_buffer_allocation);
}
