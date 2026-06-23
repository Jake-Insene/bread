#include "renderer/framed_buffer.h"


void FramedBuffer::init(const FramedBufferCreateInfo& info)
{
    data.allocator = info.allocator;
    data.device = info.device;
    data.gpu_memory_allocator = info.gpu_memory_allocator;

    data.buffer_size = Mem::align_up(info.buffer_size, GPU::MinHeapResourceAlignment);
    data.buffers_info = Array<BufferInfo>::with_size(data.allocator, info.frame_count);
}

void FramedBuffer::destroy()
{
    data.buffers_info.destroy();
}

void FramedDeviceBuffer::init(const FramedBufferCreateInfo& info)
{
    FramedBuffer::init(info);
   
    data.buffer = GPU::buffer_create(
        info.device,
        GPU::BufferCreateInfo::create(
            info.usage | GPU::BufferUsage::TransferDestination,
            info.frame_count * get_buffer_size()
        )
    );
    data.staging_buffer = GPU::buffer_create(
        info.device,
        GPU::BufferCreateInfo::create(
            info.usage | GPU::BufferUsage::TransferSource,
            info.frame_count * get_buffer_size()
        )
    );

    data.buffer_allocation = info.gpu_memory_allocator->allocate(GPUMemoryAllocator::AllocationTag::Buffer,
        GPU::buffer_get_memory_requirements(data.buffer));
    data.staging_allocation = info.gpu_memory_allocator->allocate(GPUMemoryAllocator::AllocationTag::Staging,
        GPU::buffer_get_memory_requirements(data.staging_buffer));

    GPU::buffer_bind_memory_heap(data.buffer,
        GPU::BindMemoryInfo::create(
            info.gpu_memory_allocator->allocation_get_heap(data.buffer_allocation),
            info.gpu_memory_allocator->allocation_get_offset(data.buffer_allocation)
        )
    );

    GPU::buffer_bind_memory_heap(data.staging_buffer, 
        GPU::BindMemoryInfo::create(
            info.gpu_memory_allocator->allocation_get_heap(data.staging_allocation),
            info.gpu_memory_allocator->allocation_get_offset(data.staging_allocation)
        )
    );

    for(usize i = 0; i < info.frame_count; i++)
    {
        (void)Base::data.buffers_info.add(
            {
                .offset = i * get_buffer_size(),
            }
        );
    }

    // TODO: gpu_memory_allocator->map_allocation
    data.mapped_staging = GPU::memory_heap_map(info.gpu_memory_allocator->allocation_get_heap(data.staging_allocation),
        info.gpu_memory_allocator->allocation_get_offset(data.staging_allocation),
        info.frame_count * get_buffer_size());
}

void FramedDeviceBuffer::destroy()
{
    GPU::memory_heap_unmap(get_gpu_memory_allocator()->allocation_get_heap(data.staging_allocation), data.mapped_staging);
    GPU::buffer_destroy(data.buffer);
    GPU::buffer_destroy(data.staging_buffer);
    get_gpu_memory_allocator()->free(data.buffer_allocation);
    get_gpu_memory_allocator()->free(data.staging_allocation);

    FramedBuffer::destroy();
}

void FramedMappedBuffer::init(const FramedBufferCreateInfo& info)
{
    FramedBuffer::init(info);

    data.mapped_buffer = GPU::buffer_create(
        info.device,
        GPU::BufferCreateInfo::create(
            info.usage,
            info.frame_count * get_buffer_size()
        )
    );

    data.mapped_buffer_allocation = info.gpu_memory_allocator->allocate(GPUMemoryAllocator::AllocationTag::Staging,
        GPU::buffer_get_memory_requirements(data.mapped_buffer));
    GPU::buffer_bind_memory_heap(data.mapped_buffer, 
        GPU::BindMemoryInfo::create(
            info.gpu_memory_allocator->allocation_get_heap(data.mapped_buffer_allocation),
            info.gpu_memory_allocator->allocation_get_offset(data.mapped_buffer_allocation)
        )
    );

    for(usize i = 0; i < info.frame_count; i++)
    {
        (void)Base::data.buffers_info.add(
            {
                .offset = i * get_buffer_size(),
            }
        );
    }

    // TODO: gpu_memory_allocator->map_allocation
    data.mapped = GPU::memory_heap_map(info.gpu_memory_allocator->allocation_get_heap(data.mapped_buffer_allocation),
        info.gpu_memory_allocator->allocation_get_offset(data.mapped_buffer_allocation),
        info.frame_count * get_buffer_size());
}

void FramedMappedBuffer::destroy()
{
    GPU::memory_heap_unmap(get_gpu_memory_allocator()->allocation_get_heap(data.mapped_buffer_allocation), data.mapped);
    GPU::buffer_destroy(data.mapped_buffer);
    get_gpu_memory_allocator()->free(data.mapped_buffer_allocation);
    FramedBuffer::destroy();
}
