#include "render_device/core/gpu_memory_allocator.h"

#include "debug/fail.h"


void GPUMemoryAllocator::init(const GPUMemoryAllocatorCreateInfo& info)
{
    allocator = info.allocator;
    graphics_device = info.graphics_device;

    heaps = Array<Heap>::with_size(allocator, 4);
    allocations = FreeList<Allocation, GPUMemoryAllocationID>::with_size(allocator, 4);

    staging_buffer = graphics_device->create_buffer(
        GPU::BufferUsage::TransferSource, StagingHeapInitialSize
    );

    staging_heap = graphics_device->create_memory_heap(
        GPU::HeapUsage::CPUGPUCoherent, StagingHeapInitialSize
    );
    staging_buffer->bind_memory(staging_heap, 0);

    staging_heap_current_size = StagingHeapInitialSize;
    mapped_staging_heap = staging_heap->map(0, staging_heap_current_size);
}

void GPUMemoryAllocator::destroy()
{
    staging_buffer->destroy();
    staging_heap->unmap(mapped_staging_heap);
    staging_heap->destroy();
    
    for(Heap& heap : heaps.iter())
    {
        heap.heap->destroy();
    }

    heaps.destroy();
    allocations.destroy();
}

GPUMemoryAllocationID GPUMemoryAllocator::allocate(AllocationTag tag, const GPU::MemoryRequirements& requirements)
{
    usize aligned_size = Mem::align_up(requirements.size, requirements.alignment);

    GPUMemoryAllocationID allocation_id = GPUMemoryAllocationID::invalid();
    
    // First step check in the current available heaps.
    
    for(Heap& heap : heaps.iter())
    {
        if(allocation_id != GPUMemoryAllocationID::invalid())
        {
            break;
        }
        
        GPUMemoryAllocationID alloc_current_id = heap.first_allocation;
        for(;alloc_current_id != GPUMemoryAllocationID::invalid();)
        {
            Allocation& allocation = allocations.get(alloc_current_id);
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
    allocation_id = allocations.add(new_allocation);
    new_heap.first_allocation = allocation_id;

    return allocation_id;
}

void GPUMemoryAllocator::free(GPUMemoryAllocationID allocation)
{
    FailOn(allocation.is_valid() == false, "invalid allocation");
    Allocation& alloc = allocations.get(allocation);
    alloc.free = true;
    allocations.remove(allocation);
}

Graphics::Buffer* GPUMemoryAllocator::begin_staging(usize size)
{
    Unused(size);
    return staging_buffer;
}

void GPUMemoryAllocator::end_staging(Graphics::Buffer*)
{

}

Slice<u8> GPUMemoryAllocator::map_staging()
{
    return mapped_staging_heap;
}

void GPUMemoryAllocator::unmap_staging(const Slice<u8>& memory)
{
    Unused(memory);
}

Graphics::MemoryHeap* GPUMemoryAllocator::allocation_get_heap(GPUMemoryAllocationID allocation)
{
    FailOn(allocation.is_valid() == false, "invalid allocation");

    return heaps.get(allocations.get(allocation).heap_index).heap;
}

[[nodiscard]] usize GPUMemoryAllocator::allocation_get_offset(GPUMemoryAllocationID allocation)
{
    FailOn(allocation.is_valid() == false, "invalid allocation");

    return allocations.get(allocation).offset;
}

GPUMemoryAllocator::Heap& GPUMemoryAllocator::_request_heap_for(AllocationTag tag, usize size, GPU::HeapUsage heap_usage)
{
    for(Heap& heap : heaps.iter())
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
        .heap = graphics_device->create_memory_heap(required_heap_usage, heap_size),
        .heap_size = heap_size,
        .heap_usage = required_heap_usage,
        .tag = tag,
        .heap_index = heaps.count,
        .first_allocation = GPUMemoryAllocationID::invalid(),
    };

    return heaps.add(new_heap);
}



