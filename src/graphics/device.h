#pragma once
#include "display/window.h"
#include "mem/allocator.h"
#include "os/mutex.h"
#include "gpu/gpu.h"
#include "graphics/buffer.h"
#include "graphics/command_queue.h"
#include "graphics/descriptor_pool.h"
#include "graphics/device_object.h"
#include "graphics/fence.h"
#include "graphics/memory_heap.h"
#include "graphics/pipeline_layout.h"
#include "graphics/pipeline.h"
#include "graphics/queue.h"
#include "graphics/sampler.h"
#include "graphics/semaphore.h"
#include "graphics/swap_chain.h"
#include "graphics/texture.h"


namespace Graphics
{

struct Device
{
    mem::Allocator* allocator;
    Mutex mutex;

    GPU::PhysicalDeviceID gpu_physical_device;
    GPU::DeviceID gpu_device;

    Queue graphics_queue;
    Queue compute_queue;
    Queue copy_queue;
    Queue present_queue;

    Array<DeviceObject*> allocated_objects;

    Queue* get_graphics_queue() { return &graphics_queue; }
    Queue* get_compute_queue() { return &compute_queue; }
    Queue* get_copy_queue() { return &copy_queue; }
    Queue* get_present_queue() { return &present_queue; }

    void init(mem::Allocator* _allocator, GPU::PhysicalDeviceID _gpu_physical_device);
    void destroy();

    SwapChain* create_swap_chain(Window* window, GPU::TextureFormat surface_format);
    Fence* create_fence(bool signaled);
    Semaphore* create_semaphore();
    MemoryHeap* create_memory_heap(GPU::HeapUsage usage, usize size);
    Buffer* create_buffer(GPU::BufferUsage usage, usize size);
    Sampler* create_sampler(const SamplerInfo& sampler_info);
    Texture* create_texture(const TextureInfo& texture_info);
    DescriptorPool* create_descriptor_pool(u32 max_sets, Slice<const GPU::DescriptorPoolSize> sizes);
    PipelineLayout* create_pipeline_layout(const PipelineLayoutInfo& pipeline_layout_info);
    Pipeline* create_pipeline(const PipelineInfo& pipeline_info);
    CommandQueue* create_command_queue(Queue* queue);

    void release_object(DeviceObject* child);

    void _log_child_alloc(DeviceObject* child);

    template<typename T>
    requires(IsBaseOf<DeviceObject, T>)
    T* _allocate_object()
    {
        T* object = allocator->object<T>();
        _log_child_alloc(static_cast<DeviceObject*>(object));
        (void)allocated_objects.add(
            static_cast<DeviceObject*>(object)
        );
        return object;
    }
};

}
