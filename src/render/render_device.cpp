#include "render/render_device.h"

#include "engine/engine.h"
#include "log/log.h"


void RenderDevice::initialize(const SystemInitializeInfo&)
{
    gpu_device = GPU::device_create(
        {
            .physical_device = Engine::get_selected_gpu_device(),
        }
    );

    graphics_queue = GPU::queue_create(
        {
            .device = gpu_device,
            .usage = GPU::QueueUsage::Graphics,
        }
    );

    compute_queue = GPU::queue_create(
        {
            .device = gpu_device,
            .usage = GPU::QueueUsage::Compute,
        }
    );

    copy_queue = GPU::queue_create(
        {
            .device = gpu_device,
            .usage = GPU::QueueUsage::Copy,
        }
    );

    present_queue = GPU::queue_create(
        {
            .device = gpu_device,
            .usage = GPU::QueueUsage::Present,
        }
    );
}

void RenderDevice::shutdown()
{
    GPU::queue_wait_idle(graphics_queue);
    GPU::queue_wait_idle(compute_queue);
    GPU::queue_wait_idle(copy_queue);
    GPU::queue_wait_idle(present_queue);

    GPU::device_destroy(gpu_device);

    GPU::queue_destroy(graphics_queue);
    GPU::queue_destroy(compute_queue);
    GPU::queue_destroy(copy_queue);
    GPU::queue_destroy(present_queue);
}
