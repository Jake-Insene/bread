#pragma once
#include "mem/allocator.h"
#include "gpu/gpu.h"
#include "graphics/buffer.h"
#include "graphics/command_queue.h"
#include "graphics/descriptor_pool.h"
#include "graphics/device_object.h"
#include "graphics/fence.h"
#include "graphics/memory_heap.h"
#include "graphics/pipeline.h"
#include "graphics/queue.h"
#include "graphics/sampler.h"
#include "graphics/semaphore.h"
#include "graphics/swap_chain.h"


namespace Graphics
{

struct Device
{
    mem::Allocator allocator;

    GPU::PhysicalDeviceID gpu_physical_device;
    GPU::DeviceID gpu_device;

    Queue graphics_queue;
    Queue compute_queue;
    Queue copy_queue;
    Queue present_queue;

    Array<DeviceObject*> allocated_objects;

    Queue& get_graphics_queue() { return graphics_queue; }
    Queue& get_compute_queue() { return compute_queue; }
    Queue& get_copy_queue() { return copy_queue; }
    Queue& get_present_queue() { return present_queue; }

    void init(const mem::Allocator& _allocator, GPU::PhysicalDeviceID _gpu_physical_device);
    void destroy();

    Ptr<SwapChain> create_swap_chain(Window window, GPU::TextureFormat surface_format);
    Ptr<Fence> create_fence(bool signaled);
    Ptr<Semaphore> create_semaphore();
    Ptr<MemoryHeap> create_memory_heap(GPU::HeapUsage usage, usize size);
    Ptr<Buffer> create_buffer(GPU::BufferUsage usage, usize size, Ptr<MemoryHeap> heap, usize heap_offset);
    Ptr<Sampler> create_sampler(const SamplerInfo& sampler_info);
    Ptr<DescriptorPool> create_descriptor_pool(u32 max_sets, Slice<const GPU::DescriptorPoolSize> sizes);
    Ptr<Pipeline> create_pipeline(const PipelineInfo& pipeline_info);
    Ptr<CommandQueue> create_command_queue(Queue& queue);

    void release_object(DeviceObject* child);

    void _log_child_alloc(DeviceObject* child);

    template<typename T>
    requires(IsBaseOf<DeviceObject, T>)
    Ptr<T> _allocate_object()
    {
        T* object = allocator.object<T>();
        _log_child_alloc(static_cast<DeviceObject*>(object));
        (void)allocated_objects.add(
            static_cast<DeviceObject*>(object)
        );
        return Ptr<T>::from_memory(allocator, object);
    }
};

}
