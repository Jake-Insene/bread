#include "graphics/gpu_memory_allocator.h"

#include "debug/fail.h"


namespace Graphics
{

void GPUMemoryAllocator::init(const GPUMemoryAllocatorCreateInfo& info)
{
    data.allocator = info.allocator;
    data.device = info.device;
    data.graphics_queue = info.graphics_queue;
    data.copy_queue = info.copy_queue;

    data.heaps = Array<Heap>::with_size(data.allocator, 4);
    data.allocations = FreeList<Allocation, GPUMemoryAllocationID>::with_size(data.allocator, 4);
    data.staging_heaps = Array<StagingHeap>::with_size(data.allocator, 4);
}

void GPUMemoryAllocator::destroy()
{
    (void)data.staging_heaps.iter().for_each([](StagingHeap& staging_heap)
    {
        if(HasValue(staging_heap.flags & StagingFlags::Mapped))
        {
            GPU::memory_heap_unmap(staging_heap.heap, staging_heap.mapped_buffer);
        }
        GPU::buffer_destroy(staging_heap.buffer);
        GPU::memory_heap_destroy(staging_heap.heap);
    });

    (void)data.heaps.iter().for_each([](Heap& heap)
    {
        GPU::memory_heap_destroy(heap.heap);

    });

    data.heaps.destroy();
    data.allocations.destroy();
    data.staging_heaps.destroy();
}

GPUMemoryAllocationID GPUMemoryAllocator::allocate(AllocationTag tag, const GPU::MemoryRequirements& requirements)
{
    usize aligned_size = Mem::align_up(requirements.size, requirements.alignment);

    GPUMemoryAllocationID allocation_id = GPUMemoryAllocationID::invalid();
    
    // First step check in the current available heaps.
    for(Heap& heap : data.heaps.iter())
    {
        if(allocation_id != GPUMemoryAllocationID::invalid())
        {
            break;
        }
        
        GPUMemoryAllocationID alloc_current_id = heap.first_allocation;
        for(;alloc_current_id != GPUMemoryAllocationID::invalid();)
        {
            Allocation& allocation = data.allocations.get(alloc_current_id);
            if(!allocation.free)
            {
                alloc_current_id = allocation.next;
                continue;
            }
            
            if(allocation.size >= aligned_size)
            {
                allocation_id = alloc_current_id;
                break;
            }        
        }
    }

    // TODO: creating always a heap.
    Heap& new_heap = _create_heap(tag, aligned_size, requirements.heap_usage);
    Allocation new_allocation =
    {
        .heap_index = new_heap.heap_index,
        .heap_offset = 0,
        .tag = tag,
        .size = new_heap.heap_size,
        .free = false,
        .prev = GPUMemoryAllocationID::invalid(),
        .next = GPUMemoryAllocationID::invalid(),
    };
    allocation_id = data.allocations.add(new_allocation);
    new_heap.first_allocation = allocation_id;

    return allocation_id;
}

void GPUMemoryAllocator::free(GPUMemoryAllocationID allocation)
{
    FailOn(allocation.is_valid() == false, "invalid allocation");
    Allocation& alloc = data.allocations.get(allocation);
    alloc.free = true;
    data.allocations.remove(allocation);
}

GPU::BufferID GPUMemoryAllocator::begin_staging(usize size)
{
    GPU::BufferID buffer = GPU::BufferID::invalid();
    for(StagingHeap& staging_heap : data.staging_heaps.iter())
    {
        if(HasValue(staging_heap.flags & StagingFlags::Allocated))
        {
            continue;
        }

        if(staging_heap.heap_size < size)
        {
            continue;
        }

        staging_heap.flags |= StagingFlags::Allocated;
        buffer = staging_heap.buffer;
    }

    if(buffer == GPU::BufferID::invalid())
    {
        buffer = GPU::buffer_create(data.device,
            GPU::BufferCreateInfo::create(GPU::BufferUsage::TransferSource, size));

        GPU::MemoryRequirements requirements = GPU::buffer_get_memory_requirements(buffer);
        usize heap_size = Mem::align_up(requirements.size, requirements.alignment);

        GPU::MemoryHeapID new_heap = GPU::memory_heap_create(
            data.device,
            {
                .heap_usage = GPU::HeapUsage::CPUGPUCoherent,
                .heap_size = heap_size,
            }
        );
        GPU::buffer_bind_memory_heap(buffer, GPU::BindMemoryInfo::create(new_heap, 0));

        (void)data.staging_heaps.add(
            StagingHeap
            {
                .heap = new_heap,
                .flags = StagingFlags(),
                .heap_size = heap_size,
                .buffer = buffer,
                .mapped_buffer = Slice<u8>(),
            }
        );
    }

    return buffer;
}

void GPUMemoryAllocator::end_staging(GPU::BufferID staging_buffer)
{
    for(StagingHeap& staging_heap : data.staging_heaps.iter())
    {
        if(staging_heap.buffer == staging_buffer)
        {
            staging_heap.flags ^= StagingFlags::Allocated;
            return;
        }
    }
}

Slice<u8> GPUMemoryAllocator::map_staging(GPU::BufferID staging_buffer)
{
    for(StagingHeap& staging_heap : data.staging_heaps.iter())
    {
        if(staging_heap.buffer != staging_buffer)
        {
            continue;
        }

        if(!HasValue(staging_heap.flags & StagingFlags::Mapped))
        {
            staging_heap.mapped_buffer = GPU::memory_heap_map(staging_heap.heap, 0, staging_heap.heap_size);
            staging_heap.flags |= StagingFlags::Mapped;
        }

        return staging_heap.mapped_buffer;
    }

    return Slice<u8>();
}

void GPUMemoryAllocator::unmap_staging(GPU::BufferID staging_buffer, const Slice<u8>& memory)
{
    for(StagingHeap& staging_heap : data.staging_heaps.iter())
    {
        if(staging_heap.buffer != staging_buffer)
        {
            continue;
        }

        if(HasValue(staging_heap.flags & StagingFlags::Mapped))
        {
            GPU::memory_heap_unmap(staging_heap.heap, memory);
            staging_heap.flags ^= StagingFlags::Mapped;
        }
    }
}

GPU::MemoryHeapID GPUMemoryAllocator::allocation_get_heap(GPUMemoryAllocationID allocation)
{
    FailOn(allocation.is_valid() == false, "invalid allocation");
    Allocation& allocation_data = data.allocations.get(allocation);
    FailOn(allocation_data.free == true, "use after free");

    return data.heaps.get(allocation_data.heap_index).heap;
}

[[nodiscard]] usize GPUMemoryAllocator::allocation_get_offset(GPUMemoryAllocationID allocation)
{
    FailOn(allocation.is_valid() == false, "invalid allocation");
    Allocation& allocation_data = data.allocations.get(allocation);

    FailOn(allocation_data.free == true, "use after free");
    return allocation_data.heap_offset;
}

Slice<u8> GPUMemoryAllocator::allocation_map(GPUMemoryAllocationID allocation)
{
    FailOn(allocation.is_valid() == false, "invalid allocation");
    Allocation& allocation_data = data.allocations.get(allocation);
    Heap& heap = data.heaps.get(allocation_data.heap_index);
    FailOn(heap.tag != AllocationTag::Staging, "can't map this kind of allocation");

    heap.map_count++;
    if(heap.mapped.null())
    {
        heap.mapped = GPU::memory_heap_map(heap.heap, 0, heap.heap_size);
    }

    return heap.mapped.add(allocation_data.heap_offset).slice(allocation_data.size);
}

void GPUMemoryAllocator::allocation_unmap(GPUMemoryAllocationID allocation, const Slice<u8>& mapped)
{
    Unused(mapped);

    FailOn(allocation.is_valid() == false, "invalid allocation");
    Allocation& allocation_data = data.allocations.get(allocation);
    Heap& heap = data.heaps.get(allocation_data.heap_index);
    FailOn(heap.tag != AllocationTag::Staging, "can't unmap this kind of allocation");  

    heap.map_count--;
    if(heap.map_count == 0)
    {
        GPU::memory_heap_unmap(heap.heap, heap.mapped);
    }
}

GPUMemoryAllocator::Heap& GPUMemoryAllocator::_request_heap_for(AllocationTag tag, usize size, GPU::HeapUsage heap_usage)
{
    for(Heap& heap : data.heaps.iter())
    {
        if(heap.tag == tag && heap.heap_usage == heap_usage)
        {
            return heap;
        }
    }

    return _create_heap(tag, size, heap_usage);
}

GPUMemoryAllocator::Heap& GPUMemoryAllocator::_create_heap(AllocationTag tag, usize size, GPU::HeapUsage heap_usage)
{
    GPU::HeapUsage required_heap_usage = heap_usage;
    if(tag == AllocationTag::Staging && required_heap_usage == GPU::HeapUsage::GPUExclusive)
    {
        required_heap_usage = GPU::HeapUsage::CPUGPUCoherent;
    }

    Heap new_heap =
    {
        .heap = GPU::memory_heap_create(data.device,
            GPU::MemoryHeapCreateInfo::create(required_heap_usage, size)),
        .heap_size = size,
        .heap_usage = required_heap_usage,
        .tag = tag,
        .heap_index = data.heaps.count,
        .map_count = 0,
        .mapped = {},
        .first_allocation = GPUMemoryAllocationID::invalid(),
    };

    return data.heaps.add(new_heap);
}

}
