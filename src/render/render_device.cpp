#include "render/render_device.h"

#include "engine/engine.h"
#include "log/log.h"


void RenderDevice::initialize(const SystemInitializeInfo&)
{
    gpu_device = Graphics::device_create(
        {
            .physical_device = Engine::get_selected_gpu_device(),
        }
    );

    graphics_queue = Graphics::queue_create(
        {
            .device = gpu_device,
            .usage = Graphics::QueueUsage::Graphics,
        }
    );

    compute_queue = Graphics::queue_create(
        {
            .device = gpu_device,
            .usage = Graphics::QueueUsage::Compute,
        }
    );

    copy_queue = Graphics::queue_create(
        {
            .device = gpu_device,
            .usage = Graphics::QueueUsage::Copy,
        }
    );

    present_queue = Graphics::queue_create(
        {
            .device = gpu_device,
            .usage = Graphics::QueueUsage::Present,
        }
    );
}

void RenderDevice::shutdown()
{
    Graphics::queue_wait_idle(graphics_queue);
    Graphics::queue_wait_idle(compute_queue);
    Graphics::queue_wait_idle(copy_queue);
    Graphics::queue_wait_idle(present_queue);

    Graphics::device_destroy(gpu_device);

    Graphics::queue_destroy(graphics_queue);
    Graphics::queue_destroy(compute_queue);
    Graphics::queue_destroy(copy_queue);
    Graphics::queue_destroy(present_queue);
}
