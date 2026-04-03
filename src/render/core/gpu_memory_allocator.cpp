#include "render/core/gpu_memory_allocator.h"

#include "engine/engine.h"
#include "render/render_device.h"


void GPUMemoryAllocator::initialize(const mem::Allocator& _allocator)
{
    allocator = _allocator;

    render_device = Engine::get_system_manager()->get_system<RenderDevice>();

    heaps = Array<Heap>::with_size(allocator, 4);
    allocations = FreeList<Allocation, GPUMemoryAllocationID>::with_size(allocator, 4);

    staging_heap = render_device->get_graphics_device().create_memory_heap(
        GPU::HeapUsage::CPUGPUCoherent, StagingHeapInitialSize
    ).get();

    staging_buffer = render_device->get_graphics_device().create_buffer(
        GPU::BufferUsage::TransferSource, StagingHeapInitialSize, Ptr<Graphics::MemoryHeap>::from_raw(staging_heap), 0
    ).get();

    staging_heap_current_size = StagingHeapInitialSize;
    mapped_staging_heap = staging_heap->map(0, staging_heap_current_size);
}

void GPUMemoryAllocator::shutdown()
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

GPUMemoryAllocationID GPUMemoryAllocator::allocate(AllocationTag tag, usize size)
{
    usize aligned_size = mem::align_up(size, GPU::MinHeapResourceAlignment);

    GPUMemoryAllocationID allocation_id = GPUMemoryAllocationID::invalid();
    
    // First step check in the current available heaps.
    
    for(Heap& heap : heaps.iter())
    {
        if(allocation_id != GPUMemoryAllocationID::invalid()) break;
        
        GPUMemoryAllocationID alloc_current_id = heap.first_allocation;
        for(;alloc_current_id != GPUMemoryAllocationID::invalid();)
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

Ptr<Graphics::Buffer> GPUMemoryAllocator::begin_staging(usize size)
{
    Unused(size);
    return Ptr<Graphics::Buffer>::from_raw(staging_buffer);
}

void GPUMemoryAllocator::end_staging(Ptr<Graphics::Buffer>)
{

}

Slice<u8> GPUMemoryAllocator::map_staging()
{
    return mapped_staging_heap;
}

void GPUMemoryAllocator::unmap_staging(Slice<u8> memory)
{
    Unused(memory);
}

Ptr<Graphics::MemoryHeap> GPUMemoryAllocator::allocation_get_heap(GPUMemoryAllocationID allocation)
{
    FailOn(allocation.is_valid() == false, "invalid allocation");

    return Ptr<Graphics::MemoryHeap>::from_raw(heaps.get(allocations.get(allocation).heap_index).heap);
}

[[nodiscard]] usize GPUMemoryAllocator::allocation_get_offset(GPUMemoryAllocationID allocation)
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
        .heap = render_device->get_graphics_device().create_memory_heap(
            _tag_get_gpu_usage(tag), heap_size
        ).get(),
        .heap_size = heap_size,
        .tag = tag,
        .heap_index = heaps.count,
        .first_allocation = GPUMemoryAllocationID::invalid(),
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

