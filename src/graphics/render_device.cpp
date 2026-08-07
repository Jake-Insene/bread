#include "graphics/render_device.h"

#include "engine/engine.h"


namespace Graphics
{

static RenderDevice::QueueList get_queue_list(GPU::DeviceID device)
{
    RenderDevice::QueueList queues = {};

    // Garanted
    queues.graphics = GPU::queue_get(device, {.usage = GPU::QueueUsage::Graphics, .index = 0});

    if(GPU::queue_get_count(device, {.usage = GPU::QueueUsage::Compute}) > 0)
    {
        queues.compute = GPU::queue_get(device, {.usage = GPU::QueueUsage::Compute, .index = 0});
    }
    else
    {
        queues.compute = queues.graphics;
    }

    if(GPU::queue_get_count(device, {.usage = GPU::QueueUsage::Copy}) > 0)
    {
        queues.copy = GPU::queue_get(device, {.usage = GPU::QueueUsage::Copy, .index = 0});
    }
    else
    {
        queues.copy = queues.compute;
    }

    if(GPU::queue_get_count(device, {.usage = GPU::QueueUsage::Present}) > 0)
    {
        queues.present = GPU::queue_get(device, {.usage = GPU::QueueUsage::Present, .index = 0});
    }
    else
    {
        queues.present = queues.graphics;
    }

    return queues;
}

RenderDevice::RenderDevice(Mem::Allocator& allocator)
: allocator(allocator),
physical_device(Engine::get_selected_gpu_device()),
device(GPU::device_create(physical_device, {})),
queues(get_queue_list(device))
{}

RenderDevice::~RenderDevice()
{
    GPU::queue_wait_idle(get_graphics_queue());
    GPU::queue_wait_idle(get_compute_queue());
    GPU::queue_wait_idle(get_copy_queue());
    GPU::queue_wait_idle(get_present_queue());

    GPU::device_destroy(device);
}

}
