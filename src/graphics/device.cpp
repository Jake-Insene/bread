#include "graphics/device.h"


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
}

void Device::destroy()
{
    graphics_queue.wait_idle();
    compute_queue.wait_idle();
    copy_queue.wait_idle();
    present_queue.wait_idle();

    graphics_queue.destroy();
    compute_queue.destroy();
    copy_queue.destroy();
    present_queue.destroy();

    GPU::device_destroy(gpu_device);
}

SwapChain Device::create_swap_chain(Window window, GPU::SurfaceFormat surface_format)
{
    SwapChain sc = {};
    sc.init(allocator,
        {
            .device = gpu_device,
            .present_queue = Ptr<Queue>::from_raw(&present_queue),
            .window = window,
            .surface_format = surface_format,
        }
    );
    return sc;
}

MemoryHeap Device::create_memory_heap(GPU::HeapUsage usage, usize size)
{
    MemoryHeap heap = {};
    heap.init(allocator,
        {
            .device = gpu_device,
            .heap_usage = usage,
            .heap_size = size,
        }
    );
    return heap;
}

Buffer Device::create_buffer(GPU::BufferUsage usage, usize size, Ptr<MemoryHeap> heap, usize heap_offset)
{
    Buffer buffer = {};
    buffer.init(allocator,
        {
            .device = gpu_device,
            .usage = usage,
            .size = size,
            .heap = heap,
            .heap_offset = heap_offset,
        }
    );
    return buffer;
}

Sampler Device::create_sampler(const SamplerInfo& sampler_info)
{
    Sampler sampler = {};
    sampler.init(allocator, gpu_device, sampler_info);
    return sampler;
}

DescriptorPool Device::create_descriptor_pool(u32 max_sets, Slice<const GPU::DescriptorPoolSize> sizes)
{
    DescriptorPool descriptor_pool = {};
    descriptor_pool.init(allocator,
        {
            .device = gpu_device,
            .max_sets = max_sets,
            .sizes = sizes,
        }
    );
    return descriptor_pool;
}

Pipeline Device::create_pipeline(const PipelineInfo& pipeline_info)
{
    Pipeline pipe = {};
    pipe.init(allocator, gpu_device, pipeline_info);
    return pipe;
}

CommandQueue Device::create_command_queue(Queue& queue)
{
    CommandQueue command_queue = {};
    command_queue.init(allocator,
        {
            .device = gpu_device,
            .queue = queue.gpu_queue,
        }
    );
    return command_queue;
}

}