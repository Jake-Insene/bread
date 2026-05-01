#pragma once
#include "collections/array.h"
#include "collections/free_list.h"
#include "gpu/gpu.h"
#include "graphics/buffer.h"
#include "graphics/device.h"
#include "graphics/memory_heap.h"
#include "mem/allocator.h"
#include "render_device/core/gpu_memory_allocator_types.h"



struct GPUMemoryAllocatorCreateInfo
{
    mem::Allocator allocator;
    Graphics::Device* graphics_device;
};


struct GPUMemoryAllocator
{
    // 4mb
    static constexpr usize StagingHeapInitialSize = 1024 * 1024 * 4;

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
        Graphics::MemoryHeap* heap;
        usize heap_size;
        GPU::HeapUsage heap_usage;
        AllocationTag tag;
        usize heap_index;

        GPUMemoryAllocationID first_allocation;
    };

    mem::Allocator allocator;
    Graphics::Device* graphics_device;
    Array<Heap> heaps;
    FreeList<Allocation, GPUMemoryAllocationID> allocations;
    Graphics::MemoryHeap* staging_heap;
    Graphics::Buffer* staging_buffer;
    usize staging_heap_current_size;
    Slice<u8> mapped_staging_heap;

    void init(const GPUMemoryAllocatorCreateInfo& info);
    void destroy();

    GPUMemoryAllocationID allocate(AllocationTag tag, const GPU::MemoryRequirements& requirements);
    void free(GPUMemoryAllocationID allocation);

    Graphics::Buffer* begin_staging(usize size);
    void end_staging(Graphics::Buffer* staging_buffer);
    Slice<u8> map_staging();
    void unmap_staging(const Slice<u8>& memory);

    Graphics::MemoryHeap* allocation_get_heap(GPUMemoryAllocationID allocation);
    [[nodiscard]] usize allocation_get_offset(GPUMemoryAllocationID allocation);

    Heap& _request_heap_for(AllocationTag tag, usize size, GPU::HeapUsage heap_usage);
    Heap& _create_heap(AllocationTag tag, usize size, GPU::HeapUsage heap_usage);
};
