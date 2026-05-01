#include "graphics/device.h"

#include "log/log.h"


namespace Graphics
{

void Device::init(const mem::Allocator& _allocator, GPU::PhysicalDeviceID _gpu_physical_device)
{
    allocator = _allocator;
    mutex = Mutex::create();
    mutex.lock();

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

    mutex.unlock();
}

void Device::destroy()
{
    mutex.lock();

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

    mutex.unlock();

    mutex.destroy();

    GPU::device_destroy(gpu_device);
}

SwapChain* Device::create_swap_chain(Window window, GPU::TextureFormat surface_format)
{
    mutex.lock();
    SwapChain* sc = _allocate_object<SwapChain>();
    sc->init(allocator, this,
        {
            .gpu_device = gpu_device,
            .present_queue = &present_queue,
            .window = window,
            .surface_format = surface_format,
        }
    );
    mutex.unlock();
    return sc;
}

Fence* Device::create_fence(bool signaled)
{
    mutex.lock();
    Fence* fence = _allocate_object<Fence>();
    fence->init(allocator, this, gpu_device, signaled);
    mutex.unlock();
    return fence;
}

Semaphore* Device::create_semaphore()
{
    mutex.lock();
    Semaphore* semaphore = _allocate_object<Semaphore>();
    semaphore->init(allocator, this, gpu_device);
    mutex.unlock();
    return semaphore;
}

MemoryHeap* Device::create_memory_heap(GPU::HeapUsage usage, usize size)
{
    mutex.lock();
    MemoryHeap* heap = _allocate_object<MemoryHeap>();
    heap->init(allocator, this,
        {
            .device = gpu_device,
            .heap_usage = usage,
            .heap_size = size,
        }
    );
    mutex.unlock();
    return heap;
}

Buffer* Device::create_buffer(GPU::BufferUsage usage, usize size)
{
    mutex.lock();
    Buffer* buffer = _allocate_object<Buffer>();
    buffer->init(allocator, this,
        {
            .gpu_device = gpu_device,
            .usage = usage,
            .size = size,
        }
    );
    mutex.unlock();
    return buffer;
}

Sampler* Device::create_sampler(const SamplerInfo& sampler_info)
{
    mutex.lock();
    Sampler* sampler = _allocate_object<Sampler>();
    sampler->init(allocator, this, gpu_device, sampler_info);
    mutex.unlock();
    return sampler;
}

Texture* Device::create_texture(const TextureInfo& texture_info)
{
    mutex.lock();
    Texture* texture = _allocate_object<Texture>();
    texture->init(allocator, this, gpu_device, texture_info);
    mutex.unlock();
    return texture;
}

DescriptorPool* Device::create_descriptor_pool(u32 max_sets, Slice<const GPU::DescriptorPoolSize> sizes)
{
    mutex.lock();
    DescriptorPool* descriptor_pool = _allocate_object<DescriptorPool>();
    descriptor_pool->init(allocator, this,
        {
            .device = gpu_device,
            .max_sets = max_sets,
            .sizes = sizes,
        }
    );
    mutex.unlock();
    return descriptor_pool;
}

PipelineLayout* Device::create_pipeline_layout(const PipelineLayoutInfo& pipeline_layout_info)
{
    mutex.lock();
    PipelineLayout* pipe_layout = _allocate_object<PipelineLayout>();
    pipe_layout->init(allocator, this, gpu_device, pipeline_layout_info);
    mutex.unlock();
    return pipe_layout;
}

Pipeline* Device::create_pipeline(const PipelineInfo& pipeline_info)
{
    mutex.lock();
    Pipeline* pipe = _allocate_object<Pipeline>();
    pipe->init(allocator, this, gpu_device, pipeline_info);
    mutex.unlock();
    return pipe;
}

CommandQueue* Device::create_command_queue(Queue& queue)
{
    mutex.lock();
    CommandQueue* command_queue = _allocate_object<CommandQueue>();
    command_queue->init(allocator, this,
        {
            .gpu_device = gpu_device,
            .gpu_queue = queue.gpu_queue,
        }
    );
    mutex.unlock();
    return command_queue;
}

void Device::release_object(DeviceObject* child)
{
    mutex.lock();
    Log::debug("[Graphics::Device({})]: Releasing child object({})", this, child);
    DebugAssert(
        allocated_objects.iter().find(child) != allocated_objects.iter().end(),
        "the allocated object it's not owned by this device"
    );
    allocated_objects.remove(child);
    allocator.free(Slice(reinterpret_cast<u8*>(child), 1));
    mutex.unlock();
}

void Device::_log_child_alloc(DeviceObject* child)
{
    Log::debug("[Graphics::Device({})]: Allocating child object({})", this, child);
}

}