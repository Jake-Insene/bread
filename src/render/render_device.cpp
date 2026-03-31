#include "render/render_device.h"

#include "engine/engine.h"
#include "log/log.h"


void RenderDevice::initialize(const SystemInitializeInfo& info)
{
    allocator = info.allocator;

    device.init(allocator, Engine::get_selected_gpu_device());

    memory_allocator.initialize(allocator);
    resource_manager.initialize(allocator);
}

void RenderDevice::shutdown()
{
    device.get_graphics_queue().wait_idle();
    device.get_compute_queue().wait_idle();
    device.get_copy_queue().wait_idle();
    device.get_present_queue().wait_idle();

    resource_manager.shutdown();
    memory_allocator.shutdown();

    device.destroy();
}

void RenderDevice::_submit_and_wait(GPU::QueueID queue, void* arg, SubmitFn recorder)
{
    GPU::CommandPoolID pool = GPU::command_pool_create(
        {
            .device = get_graphics_device().gpu_device, .queue = queue
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
