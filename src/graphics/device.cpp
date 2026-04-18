#include "graphics/device.h"

#include "log/log.h"


namespace Graphics
{

void Device::init(const mem::Allocator& _allocator, GPU::PhysicalDeviceID _gpu_physical_device)
{
    allocator = _allocator;
    gpu_physical_device = _gpu_physical_device;

    gpu_device = GPU::device_create(
        {
            .physical_device = gpu_physical_device,
        }
    );

    graphics_queue.init(
        allocator,
        {
            .device = gpu_device,
            .usage = GPU::QueueUsage::Graphics,
        }
    );

    compute_queue.init(
        allocator,
        {
            .device = gpu_device,
            .usage = GPU::QueueUsage::Compute,
        }
    );

    copy_queue.init(
        allocator,
        {
            .device = gpu_device,
            .usage = GPU::QueueUsage::Copy,
        }
    );

    present_queue.init(
        allocator,
        {
            .device = gpu_device,
            .usage = GPU::QueueUsage::Present,
        }
    );

    allocated_objects = Array<DeviceObject*>::with_size(allocator, 4);
}

void Device::destroy()
{
    graphics_queue.wait_idle();
    compute_queue.wait_idle();
    copy_queue.wait_idle();
    present_queue.wait_idle();

    for(DeviceObject*& allocated_object : allocated_objects.iter())
    {
        allocator.free(Slice(reinterpret_cast<u8*>(allocated_object), 1));
    }
    allocated_objects.destroy();

    graphics_queue.destroy();
    compute_queue.destroy();
    copy_queue.destroy();
    present_queue.destroy();

    GPU::device_destroy(gpu_device);
}

SwapChain* Device::create_swap_chain(Window window, GPU::TextureFormat surface_format)
{
    SwapChain* sc = _allocate_object<SwapChain>();
    sc->init(allocator, this,
        {
            .gpu_device = gpu_device,
            .present_queue = &present_queue,
            .window = window,
            .surface_format = surface_format,
        }
    );
    return sc;
}

Fence* Device::create_fence(bool signaled)
{
    Fence* fence = _allocate_object<Fence>();
    fence->init(allocator, this, gpu_device, signaled);
    return fence;   
}

Semaphore* Device::create_semaphore()
{
    Semaphore* semaphore = _allocate_object<Semaphore>();
    semaphore->init(allocator, this, gpu_device);
    return semaphore;
}

MemoryHeap* Device::create_memory_heap(GPU::HeapUsage usage, usize size)
{
    MemoryHeap* heap = _allocate_object<MemoryHeap>();
    heap->init(allocator, this,
        {
            .device = gpu_device,
            .heap_usage = usage,
            .heap_size = size,
        }
    );
    return heap;
}

Buffer* Device::create_buffer(GPU::BufferUsage usage, usize size, MemoryHeap* heap, usize heap_offset)
{
    Buffer* buffer = _allocate_object<Buffer>();
    buffer->init(allocator, this,
        {
            .gpu_device = gpu_device,
            .usage = usage,
            .size = size,
            .heap = heap,
            .heap_offset = heap_offset,
        }
    );
    return buffer;
}

Sampler* Device::create_sampler(const SamplerInfo& sampler_info)
{
    Sampler* sampler = _allocate_object<Sampler>();
    sampler->init(allocator, this, gpu_device, sampler_info);
    return sampler;
}

DescriptorPool* Device::create_descriptor_pool(u32 max_sets, Slice<const GPU::DescriptorPoolSize> sizes)
{
    DescriptorPool* descriptor_pool = _allocate_object<DescriptorPool>();
    descriptor_pool->init(allocator, this,
        {
            .device = gpu_device,
            .max_sets = max_sets,
            .sizes = sizes,
        }
    );
    return descriptor_pool;
}

Pipeline* Device::create_pipeline(const PipelineInfo& pipeline_info)
{
    Pipeline* pipe = _allocate_object<Pipeline>();
    pipe->init(allocator, this, gpu_device, pipeline_info);
    return pipe;
}

CommandQueue* Device::create_command_queue(Queue& queue)
{
    CommandQueue* command_queue = _allocate_object<CommandQueue>();
    command_queue->init(allocator, this,
        {
            .gpu_device = gpu_device,
            .gpu_queue = queue.gpu_queue,
        }
    );
    return command_queue;
}

void Device::release_object(DeviceObject* child)
{
    Log::debug("[Graphics::Device({})]: Releasing child object({})", this, child);
    DebugAssert(
        allocated_objects.iter().find(child) != allocated_objects.iter().end(),
        "the allocated object it's not owned by this device"
    );
    allocated_objects.remove(child);
    allocator.free(Slice(reinterpret_cast<u8*>(child), 1));
}

void Device::_log_child_alloc(DeviceObject* child)
{
    Log::debug("[Graphics::Device({})]: Allocating child object({})", this, child);
}

}