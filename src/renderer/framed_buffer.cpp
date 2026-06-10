#include "renderer/framed_buffer.h"


void FramedBuffer::init(const FramedBufferCreateInfo& info)
{
    allocator = info.allocator;
    device = info.device;
    gpu_memory_allocator = info.gpu_memory_allocator;

    buffer_size = Mem::align_up(info.buffer_size, GPU::MinHeapResourceAlignment);
    buffers_info = Array<BufferInfo>::with_size(info.allocator, info.frame_count);
}

void FramedBuffer::destroy()
{
    buffers_info.destroy();
}

void FramedDeviceBuffer::init(const FramedBufferCreateInfo& info)
{
    FramedBuffer::init(info);
   
    buffer = GPU::buffer_create(
        info.device,
        GPU::BufferCreateInfo::create(
            info.usage | GPU::BufferUsage::TransferDestination,
            info.frame_count * buffer_size
        )
    );
    staging_buffer = GPU::buffer_create(
        info.device,
        GPU::BufferCreateInfo::create(
            info.usage | GPU::BufferUsage::TransferSource,
            info.frame_count * buffer_size
        )
    );

    buffer_allocation = info.gpu_memory_allocator->allocate(GPUMemoryAllocator::AllocationTag::Buffer, GPU::buffer_get_memory_requirements(buffer));
    staging_allocation = info.gpu_memory_allocator->allocate(GPUMemoryAllocator::AllocationTag::Staging, GPU::buffer_get_memory_requirements(staging_buffer));

    GPU::buffer_bind_memory_heap(buffer, 
        GPU::BindMemoryInfo::bind(
            info.gpu_memory_allocator->allocation_get_heap(buffer_allocation),
            info.gpu_memory_allocator->allocation_get_offset(buffer_allocation)
        )
    );

    GPU::buffer_bind_memory_heap(staging_buffer, 
        GPU::BindMemoryInfo::bind(
            info.gpu_memory_allocator->allocation_get_heap(staging_allocation),
            info.gpu_memory_allocator->allocation_get_offset(staging_allocation)
        )
    );

    for(usize i = 0; i < info.frame_count; i++)
    {
        (void)buffers_info.add(
            {
                .offset = i * buffer_size,
            }
        );
    }

    // TODO: gpu_memory_allocator->map_allocation
    mapped_staging = GPU::memory_heap_map(info.gpu_memory_allocator->allocation_get_heap(staging_allocation),
        info.gpu_memory_allocator->allocation_get_offset(staging_allocation), info.frame_count * buffer_size);
}

void FramedDeviceBuffer::destroy()
{
    GPU::memory_heap_unmap(gpu_memory_allocator->allocation_get_heap(staging_allocation), mapped_staging);
    GPU::buffer_destroy(buffer);
    GPU::buffer_destroy(staging_buffer);
    gpu_memory_allocator->free(buffer_allocation);
    gpu_memory_allocator->free(staging_allocation);

    FramedBuffer::destroy();
}

void FramedMappedBuffer::init(const FramedBufferCreateInfo& info)
{
    FramedBuffer::init(info);

    mapped_buffer = GPU::buffer_create(
        info.device,
        GPU::BufferCreateInfo::create(
            info.usage,
            info.frame_count * buffer_size
        )
    );

    mapped_buffer_allocation = info.gpu_memory_allocator->allocate(GPUMemoryAllocator::AllocationTag::Staging, GPU::buffer_get_memory_requirements(mapped_buffer));
    GPU::buffer_bind_memory_heap(mapped_buffer, 
        GPU::BindMemoryInfo::bind(
            info.gpu_memory_allocator->allocation_get_heap(mapped_buffer_allocation),
            info.gpu_memory_allocator->allocation_get_offset(mapped_buffer_allocation)
        )
    );

    for(usize i = 0; i < info.frame_count; i++)
    {
        (void)buffers_info.add(
            {
                .offset = i * buffer_size,
            }
        );
    }

    // TODO: gpu_memory_allocator->map_allocation
    mapped = GPU::memory_heap_map(info.gpu_memory_allocator->allocation_get_heap(mapped_buffer_allocation),
        info.gpu_memory_allocator->allocation_get_offset(mapped_buffer_allocation), info.frame_count * buffer_size);
}

void FramedMappedBuffer::destroy()
{
    GPU::memory_heap_unmap(gpu_memory_allocator->allocation_get_heap(mapped_buffer_allocation), mapped);
    GPU::buffer_destroy(mapped_buffer);
    gpu_memory_allocator->free(mapped_buffer_allocation);
    FramedBuffer::destroy();
}
