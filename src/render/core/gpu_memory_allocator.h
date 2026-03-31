#pragma once
#include "collections/array.h"
#include "collections/free_list.h"
#include "gpu/gpu.h"
#include "graphics/buffer.h"
#include "graphics/memory_heap.h"
#include "mem/allocator.h"
#include "render/core/gpu_memory_allocator_types.h"


struct RenderDevice;


struct GPUMemoryAllocator
{
    // 4mb
    static constexpr usize StagingHeapInitialSize = 1024 * 1024 * 4;

    enum class AllocationTag
    {
        Staging,
        Texture,
        Buffer,
    };

    struct Allocation
    {
        usize heap_index;
        usize offset;

        AllocationTag tag;
        usize size;

        bool free;

        GPUMemoryAllocationID prev;
        GPUMemoryAllocationID next;
    };

    struct Heap
    {
        Graphics::MemoryHeap heap;
        usize heap_size;
        AllocationTag tag;
        usize heap_index;

        GPUMemoryAllocationID first_allocation;
    };

    mem::Allocator allocator;
    RenderDevice* render_device;
    Array<Heap> heaps;
    FreeList<Allocation, GPUMemoryAllocationID> allocations;
    Graphics::MemoryHeap staging_heap;
    Graphics::Buffer staging_buffer;
    usize staging_heap_current_size;
    Slice<u8> mapped_staging_heap;

    void initialize(const mem::Allocator& _allocator);
    void shutdown();

    GPUMemoryAllocationID allocate(AllocationTag tag, usize size);
    void free(GPUMemoryAllocationID allocation);

    Ptr<Graphics::Buffer> begin_staging(usize size);
    void end_staging(Ptr<Graphics::Buffer> staging_buffer);
    Slice<u8> map_staging();
    void unmap_staging(Slice<u8> memory);

    Ptr<Graphics::MemoryHeap> allocation_get_heap(GPUMemoryAllocationID allocation);
    [[nodiscard]] usize allocation_get_offset(GPUMemoryAllocationID allocation);

    Heap& _request_heap_for(AllocationTag tag, usize size);
    Heap& _create_heap(AllocationTag tag, usize size);

    GPU::HeapUsage _tag_get_gpu_usage(AllocationTag tag);
};
