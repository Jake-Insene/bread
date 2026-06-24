#include "graphics/gpu_memory_allocator.h"

#include "debug/fail.h"


void GPUMemoryAllocator::init(const GPUMemoryAllocatorCreateInfo& info)
{
    data.allocator = info.allocator;
    data.device = info.device;
    data.graphics_queue = info.graphics_queue;
    data.copy_queue = info.copy_queue;

    data.heaps = Array<Heap>::with_size(data.allocator, 4);
    data.allocations = FreeList<Allocation, GPUMemoryAllocationID>::with_size(data.allocator, 4);

    data.staging_buffer = GPU::buffer_create(data.device,
        GPU::BufferCreateInfo::create(GPU::BufferUsage::TransferSource, StagingHeapInitialSize)
    );

    data.staging_heap = GPU::memory_heap_create(data.device,
        GPU::MemoryHeapCreateInfo::create(
            GPU::HeapUsage::CPUGPUCoherent, StagingHeapInitialSize)
    );

    GPU::buffer_bind_memory_heap(data.staging_buffer, GPU::BindMemoryInfo::create(data.staging_heap, 0));

    data.staging_heap_current_size = StagingHeapInitialSize;
    data.mapped_staging_heap = GPU::memory_heap_map(data.staging_heap, 0, data.staging_heap_current_size);
}

void GPUMemoryAllocator::destroy()
{
    GPU::buffer_destroy(data.staging_buffer);
    GPU::memory_heap_unmap(data.staging_heap, data.mapped_staging_heap);
    GPU::memory_heap_destroy(data.staging_heap);
    
    (void)data.heaps.iter().for_each([](Heap& heap)
    {
        GPU::memory_heap_destroy(heap.heap);

    });

    data.heaps.destroy();
    data.allocations.destroy();
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
        .offset = 0,
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
    Unused(size);
    return data.staging_buffer;
}

void GPUMemoryAllocator::end_staging(GPU::BufferID)
{

}

Slice<u8> GPUMemoryAllocator::map_staging()
{
    return data.mapped_staging_heap;
}

void GPUMemoryAllocator::unmap_staging(const Slice<u8>& memory)
{
    Unused(memory);
}

GPU::MemoryHeapID GPUMemoryAllocator::allocation_get_heap(GPUMemoryAllocationID allocation)
{
    FailOn(allocation.is_valid() == false, "invalid allocation");
    return data.heaps.get(data.allocations.get(allocation).heap_index).heap;
}

[[nodiscard]] usize GPUMemoryAllocator::allocation_get_offset(GPUMemoryAllocationID allocation)
{
    FailOn(allocation.is_valid() == false, "invalid allocation");
    return data.allocations.get(allocation).offset;
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
    usize heap_size =  Mem::align_up(size, GPU::HeapAlignment);

    GPU::HeapUsage required_heap_usage = heap_usage;
    if(tag == AllocationTag::Staging && required_heap_usage == GPU::HeapUsage::GPUExclusive)
    {
        required_heap_usage = GPU::HeapUsage::CPUGPUCoherent;
    }

    Heap new_heap =
    {
        .heap = GPU::memory_heap_create(data.device,
            GPU::MemoryHeapCreateInfo::create(required_heap_usage, heap_size)),
        .heap_size = heap_size,
        .heap_usage = required_heap_usage,
        .tag = tag,
        .heap_index = data.heaps.count,
        .first_allocation = GPUMemoryAllocationID::invalid(),
    };

    return data.heaps.add(new_heap);
}



