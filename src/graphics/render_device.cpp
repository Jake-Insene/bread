#include "graphics/render_device.h"

#include "engine/engine.h"


void RenderDevice::initialize(const RenderDeviceCreateInfo& info)
{
    data.allocator = info.allocator;

    data.physical_device = Engine::get_selected_gpu_device();

    data.device = GPU::device_create(
        data.physical_device,
        {}
    );

    // Garanted
    data.queues.graphics = GPU::queue_get(data.device, {.usage = GPU::QueueUsage::Graphics, .index = 0});

    if(GPU::queue_get_count(data.device, {.usage = GPU::QueueUsage::Compute}) > 0)
    {
        data.queues.compute = GPU::queue_get(data.device, {.usage = GPU::QueueUsage::Compute, .index = 0});
    }
    else
    {
        data.queues.compute = data.queues.graphics;
    }

    if(GPU::queue_get_count(data.device, {.usage = GPU::QueueUsage::Copy}) > 0)
    {
        data.queues.copy = GPU::queue_get(data.device, {.usage = GPU::QueueUsage::Copy, .index = 0});
    }
    else
    {
        data.queues.copy = data.queues.compute;
    }

    if(GPU::queue_get_count(data.device, {.usage = GPU::QueueUsage::Present}) > 0)
    {
        data.queues.present = GPU::queue_get(data.device, {.usage = GPU::QueueUsage::Present, .index = 0});
    }
    else
    {
        data.queues.present = data.queues.graphics;
    }

    data.gpu_memory_allocator.init(
        {
            .allocator = data.allocator,
            .device = get_device(),
            .graphics_queue = get_graphics_queue(),
            .copy_queue = get_copy_queue(),
        }
    );
    data.gpu_resource_manager.init(
        {
            .allocator = data.allocator,
            .device = get_device(),
            .graphics_queue = get_graphics_queue(),
            .copy_queue = get_copy_queue(),
            .gpu_memory_allocator = get_gpu_memory_allocator(),
        }
    );
}

void RenderDevice::shutdown()
{
    GPU::queue_wait_idle(get_graphics_queue());
    GPU::queue_wait_idle(get_compute_queue());
    GPU::queue_wait_idle(get_copy_queue());
    GPU::queue_wait_idle(get_present_queue());

    data.gpu_resource_manager.destroy();
    data.gpu_memory_allocator.destroy();

    GPU::device_destroy(data.device);
}

