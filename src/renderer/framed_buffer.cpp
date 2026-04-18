#include "renderer/framed_buffer.h"

#include "graphics/buffer.h"


void FramedBuffer::init(const FramedBufferCreateInfo& info)
{
    allocator = info.allocator;
    graphics_device = info.graphics_device;
    gpu_memory_allocator = info.gpu_memory_allocator;

    buffer_size = mem::align_up(info.buffer_size, GPU::MinHeapResourceAlignment);
    buffers_info = Array<BufferInfo>::with_size(allocator, info.frame_count);
}

void FramedBuffer::destroy()
{
    buffers_info.destroy();
}

void FramedDeviceBuffer::init(const FramedBufferCreateInfo& info)
{
    FramedBuffer::init(info);
    buffer_allocation = gpu_memory_allocator->allocate(GPUMemoryAllocator::AllocationTag::Buffer, info.frame_count * buffer_size);
    staging_allocation = gpu_memory_allocator->allocate(GPUMemoryAllocator::AllocationTag::Staging, info.frame_count * buffer_size);
   
    buffer = graphics_device->create_buffer(
        info.usage | GPU::BufferUsage::TransferDestination, info.frame_count * buffer_size,
        gpu_memory_allocator->allocation_get_heap(buffer_allocation),
        gpu_memory_allocator->allocation_get_offset(buffer_allocation)
    );
    staging_buffer = graphics_device->create_buffer(
        info.usage | GPU::BufferUsage::TransferSource, info.frame_count * buffer_size,
        gpu_memory_allocator->allocation_get_heap(staging_allocation),
        gpu_memory_allocator->allocation_get_offset(staging_allocation)
    );

    for(usize i = 0; i < info.frame_count; i++)
    {
        (void)buffers_info.add(
            {
                .offset = i * buffer_size,
            }
        );
    }

    mapped_staging = staging_buffer->map(0, info.frame_count * buffer_size);
}

void FramedDeviceBuffer::destroy()
{
    staging_buffer->unmap(mapped_staging);
    buffer->destroy();
    staging_buffer->destroy();
    gpu_memory_allocator->free(buffer_allocation);
    gpu_memory_allocator->free(staging_allocation);

    FramedBuffer::destroy();
}

void FramedMappedBuffer::init(const FramedBufferCreateInfo& info)
{
    FramedBuffer::init(info);

    mapped_buffer_allocation = gpu_memory_allocator->allocate(GPUMemoryAllocator::AllocationTag::Staging, info.frame_count * buffer_size);
    mapped_buffer = graphics_device->create_buffer(
        info.usage, info.frame_count * buffer_size,
        gpu_memory_allocator->allocation_get_heap(mapped_buffer_allocation),
        gpu_memory_allocator->allocation_get_offset(mapped_buffer_allocation)
    );;

    for(usize i = 0; i < info.frame_count; i++)
    {
        (void)buffers_info.add(
            {
                .offset = i * buffer_size,
            }
        );
    }

    mapped = mapped_buffer->map(0, info.frame_count * buffer_size);
}

void FramedMappedBuffer::destroy()
{
    mapped_buffer->unmap(mapped);
    mapped_buffer->destroy();
    gpu_memory_allocator->free(mapped_buffer_allocation);
    FramedBuffer::destroy();
}
