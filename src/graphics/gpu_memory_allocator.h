#pragma once
#include "collections/array.h"
#include "collections/free_list.h"
#include "gpu/gpu.h"
#include "mem/allocator.h"
#include "graphics/gpu_memory_allocator_types.h"


namespace Graphics
{

struct GPUMemoryAllocatorCreateInfo
{
    Mem::Allocator* allocator;
    GPU::DeviceID device;
    GPU::QueueID graphics_queue;
    GPU::QueueID copy_queue;
};

struct GPUMemoryAllocator
{
    enum class AllocationTag
    {
        Staging,
        Texture,
        Buffer,
    };

    struct Allocation
    {
        usize heap_index;
        usize heap_offset;

        AllocationTag tag;
        usize size;

        bool free;

        GPUMemoryAllocationID prev;
        GPUMemoryAllocationID next;
    };

    struct Heap
    {
        GPU::MemoryHeapID heap;
        usize heap_size;
        GPU::HeapUsage heap_usage;
        AllocationTag tag;
        usize heap_index;

        usize map_count;
        Slice<u8> mapped;

        GPUMemoryAllocationID first_allocation;
    };

    enum class StagingFlags
    {
        Mapped = Bit(0),
        Allocated = Bit(1),
    };

    struct StagingHeap
    {
        GPU::MemoryHeapID heap;
        StagingFlags flags;
        usize heap_size;
        GPU::BufferID buffer;
        Slice<u8> mapped_buffer;
    };

    struct InternalData
    {
        Mem::Allocator* allocator;
        GPU::DeviceID device;
        GPU::QueueID graphics_queue;
        GPU::QueueID copy_queue;

        Array<Heap> heaps;
        FreeList<Allocation, GPUMemoryAllocationID> allocations;
        Array<StagingHeap> staging_heaps;
    } data;

    void init(const GPUMemoryAllocatorCreateInfo& info);
    void destroy();

    /*
    * Memory Allocation API
    */
    GPUMemoryAllocationID allocate(AllocationTag tag, const GPU::MemoryRequirements& requirements);
    void free(GPUMemoryAllocationID allocation);

    /*
    * Staging API
    */
    GPU::BufferID begin_staging(usize size);
    void end_staging(GPU::BufferID staging_buffer);

    Slice<u8> map_staging(GPU::BufferID staging_buffer);
    void unmap_staging(GPU::BufferID staging_buffer, const Slice<u8>& memory);

    /*
    * Allocation API
    */

    GPU::MemoryHeapID allocation_get_heap(GPUMemoryAllocationID allocation);
    [[nodiscard]] usize allocation_get_offset(GPUMemoryAllocationID allocation);
    Slice<u8> allocation_map(GPUMemoryAllocationID allocation);
    void allocation_unmap(GPUMemoryAllocationID allocation, const Slice<u8>& mapped);

    Heap& _request_heap_for(AllocationTag tag, usize size, GPU::HeapUsage heap_usage);
    Heap& _create_heap(AllocationTag tag, usize size, GPU::HeapUsage heap_usage);
};

}

EnableBitOp(Graphics::GPUMemoryAllocator::StagingFlags);
