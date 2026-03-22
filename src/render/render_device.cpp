#include "render/render_device.h"

#include "engine/engine.h"
#include "log/log.h"


void RenderDevice::initialize(const SystemInitializeInfo& info)
{
    allocator = info.allocator;

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

    memory_allocator.initialize(allocator);
    resource_manager.initialize(allocator);
}

void RenderDevice::shutdown()
{
    GPU::queue_wait_idle(graphics_queue);
    GPU::queue_wait_idle(compute_queue);
    GPU::queue_wait_idle(copy_queue);
    GPU::queue_wait_idle(present_queue);

    resource_manager.shutdown();
    memory_allocator.shutdown();

    GPU::device_destroy(gpu_device);

    GPU::queue_destroy(graphics_queue);
    GPU::queue_destroy(compute_queue);
    GPU::queue_destroy(copy_queue);
    GPU::queue_destroy(present_queue);
}

void RenderDevice::_submit_and_wait(GPU::QueueID queue, void* arg, SubmitFn recorder)
{
    GPU::CommandPoolID pool = GPU::command_pool_create(
        {
            .device = get_graphics_device(), .queue = queue
        }
    );

    GPU::CommandBufferID cmd = GPU::command_buffer_allocate(
        {
            .pool = pool,
        }
    );

    GPU::command_buffer_begin(cmd);
    recorder(arg, cmd);
    GPU::command_buffer_end(cmd);

    GPU::queue_execute_command_buffer(
        queue,
        {
            .wait_semaphores = {},
            .wait_stages = {},
            .command_buffers = Slice(&cmd, 1),
            .signal_semaphores = {},
            .fence = GPU::FenceID::invalid(),
        }
    );

    GPU::queue_wait_idle(queue);

    GPU::command_buffer_free(cmd);
    GPU::command_pool_destroy(pool);
}
