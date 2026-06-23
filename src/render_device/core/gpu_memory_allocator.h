#pragma once
#include "collections/array.h"
#include "collections/free_list.h"
#include "gpu/gpu.h"
#include "mem/allocator.h"
#include "render_device/core/gpu_memory_allocator_types.h"


struct RenderDevice;

struct GPUMemoryAllocatorCreateInfo
{
    Mem::Allocator* allocator;
    RenderDevice* render_device;
};

struct GPUMemoryAllocator
{
    // 4mb
    // TODO: Make staging incremental
    static constexpr usize StagingHeapInitialSize = 1024 * 1024 * 46;

    enum class AllocationTag
    {
        Staging,
        Texture,
        Buffer,
        MappedBuffer,
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
        GPU::MemoryHeapID heap;
        usize heap_size;
        GPU::HeapUsage heap_usage;
        AllocationTag tag;
        usize heap_index;

        GPUMemoryAllocationID first_allocation;
    };

    struct InternalData
    {
        Mem::Allocator* allocator;
        RenderDevice* render_device;

        Array<Heap> heaps;
        FreeList<Allocation, GPUMemoryAllocationID> allocations;
        GPU::MemoryHeapID staging_heap;
        GPU::BufferID staging_buffer;
        usize staging_heap_current_size;
        Slice<u8> mapped_staging_heap;
    } data;

    void init(const GPUMemoryAllocatorCreateInfo& info);
    void destroy();

    GPUMemoryAllocationID allocate(AllocationTag tag, const GPU::MemoryRequirements& requirements);
    void free(GPUMemoryAllocationID allocation);

    GPU::BufferID begin_staging(usize size);
    void end_staging(GPU::BufferID staging_buffer);
    Slice<u8> map_staging();
    void unmap_staging(const Slice<u8>& memory);

    GPU::MemoryHeapID allocation_get_heap(GPUMemoryAllocationID allocation);
    [[nodiscard]] usize allocation_get_offset(GPUMemoryAllocationID allocation);

    Heap& _request_heap_for(AllocationTag tag, usize size, GPU::HeapUsage heap_usage);
    Heap& _create_heap(AllocationTag tag, usize size, GPU::HeapUsage heap_usage);
};
