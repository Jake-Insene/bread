#include "render/core/gpu_memory_allocator.h"

#include "engine/engine.h"
#include "render/render_device.h"


void GPUMemoryAllocator::initialize(const mem::Allocator& _allocator)
{
    allocator = _allocator;

    render_device = Engine::get_system_manager().get_system<RenderDevice>();

    heaps = Array<Heap>::with_size(allocator, 4);
    allocations = FreeList<Allocation, AllocationID>::with_size(allocator, 4);

    staging_heap = GPU::memory_heap_create(
        {
            .device = render_device->get_graphics_device(),
            .heap_usage = GPU::HeapUsage::CPUGPUCoherent,
            .heap_size = StagingHeapInitialSize,
        }
    );
    
    staging_buffer = GPU::buffer_create(
        {
            .device = render_device->get_graphics_device(),
            .usage = GPU::BufferUsage::TransferSource,
            .size = StagingHeapInitialSize,
            .memory_heap = staging_heap,
            .heap_offset = 0,
        }
    );
}

void GPUMemoryAllocator::shutdown()
{
    GPU::buffer_destroy(staging_buffer);
    GPU::memory_heap_destroy(staging_heap);
    
    for(Heap& heap : heaps.iter())
    {
        GPU::memory_heap_destroy(heap.heap);
    }

    heaps.destroy();
    allocations.destroy();
}

GPUMemoryAllocator::AllocationID GPUMemoryAllocator::allocate(AllocationTag tag, usize size)
{
    usize aligned_size = mem::align_up(size, GPU::MinHeapResourceAlignment);

    AllocationID allocation_id = AllocationID::invalid();
    
    // First step check in the current available heaps.
    
    for(Heap& heap : heaps.iter())
    {
        if(allocation_id != AllocationID::invalid()) break;
        
        AllocationID alloc_current_id = heap.first_allocation;
        for(;alloc_current_id != AllocationID::invalid();)
        {
            Allocation& allocation = allocations.get(alloc_current_id);
            if(allocation.free == false)
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

    Heap& new_heap = _create_heap(tag, aligned_size);
    Allocation new_allocation =
    {
        .heap_index = new_heap.heap_index,
        .offset = 0,
        .tag = tag,
        .size = new_heap.heap_size,
        .free = false,
        .prev = AllocationID::invalid(),
        .next = AllocationID::invalid(),
    };
    allocation_id = allocations.add(new_allocation);
    new_heap.first_allocation = allocation_id;

    return allocation_id;
}

void GPUMemoryAllocator::free(AllocationID allocation)
{
    FailOn(allocation.is_valid() == false, "invalid allocation");
    Allocation& alloc = allocations.get(allocation);
    alloc.free = true;
    allocations.remove(allocation);
}

GPU::BufferID GPUMemoryAllocator::begin_staging(usize size)
{
    Unused(size);
    return staging_buffer;
}

void GPUMemoryAllocator::end_staging(GPU::BufferID)
{

}

GPU::MemoryHeapID GPUMemoryAllocator::allocation_get_heap(AllocationID allocation)
{
    FailOn(allocation.is_valid() == false, "invalid allocation");

    return heaps.get(allocations.get(allocation).heap_index).heap;
}

[[nodiscard]] usize GPUMemoryAllocator::allocation_get_offset(AllocationID allocation)
{
    FailOn(allocation.is_valid() == false, "invalid allocation");

    return allocations.get(allocation).offset;
}

GPUMemoryAllocator::Heap& GPUMemoryAllocator::_request_heap_for(AllocationTag tag, usize size)
{
    for(Heap& heap : heaps.iter())
    {
        if(heap.tag == tag)
        {
            return heap;
        }
    }

    return _create_heap(tag, size);
}

GPUMemoryAllocator::Heap& GPUMemoryAllocator::_create_heap(AllocationTag tag, usize size)
{
    usize heap_size =  mem::align_up(size, GPU::HeapAlignment);
    Heap new_heap =
    {
        .heap = GPU::memory_heap_create(
            {
                .device = render_device->get_graphics_device(),
                .heap_usage = _tag_get_gpu_usage(tag),
                .heap_size = heap_size,
            }
        ),
        .heap_size = heap_size,
        .tag = tag,
        .heap_index = heaps.count,
        .first_allocation = AllocationID::invalid(),
    };

    return heaps.add(new_heap);
}


GPU::HeapUsage GPUMemoryAllocator::_tag_get_gpu_usage(AllocationTag tag)
{
    switch(tag)
    {
    case AllocationTag::Staging:
        return GPU::HeapUsage::CPUGPUCoherent;
    case AllocationTag::Texture:
    case AllocationTag::Buffer:
        return GPU::HeapUsage::GPUExclusive;
    }

    FailOn(true, "invalid allocation tag");
    return GPU::HeapUsage();
}

