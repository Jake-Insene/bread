#include "render/command_queue.h"

#include "engine/engine.h"
#include "render/render_device.h"



void CommandQueue::initialize(const mem::Allocator& _allocator)
{
    allocator = _allocator;

    render_device = Engine::get_system_manager().get_system<RenderDevice>();

    command_pool = GPU::command_pool_create(
        {
            .device = render_device->get_graphics_device(),
            .queue = render_device->graphics_queue,
        }
    );

    encoders = Array<CommandEncoder>::with_size(allocator, 4);
    work_fences = Array<GPU::FenceID>::with_size(allocator, 4);
    work_submited = Array<WorkSubmit>::with_size(allocator, 4);

    free_fences = Stack<GPU::FenceID>::with_size(allocator, 4);
    free_encoders = Stack<CommandEncoder>::with_size(allocator, 4);
}

void CommandQueue::shutdown()
{
    for(CommandEncoder& encoder : encoders.iter())
    {
        GPU::command_buffer_free(encoder.command_buffer);
    }
    for(GPU::FenceID& fence : work_fences.iter())
    {
        GPU::fence_destroy(fence);
    }
    encoders.destroy();
    work_fences.destroy();
    work_submited.destroy();

    free_fences.destroy();
    free_encoders.destroy();

    GPU::command_pool_destroy(command_pool);
}

CommandEncoder CommandQueue::acquire_encoder()
{
    _remove_finished_work();

    if(!free_encoders.is_empty())
    {
        return free_encoders.pop();
    }

    CommandEncoder encoder =
    {
        .command_buffer = GPU::command_buffer_allocate(
            {
                .pool = command_pool,
            }
        )
    };

    return encoders.add(encoder);
}

void CommandQueue::execute(const CommandEncoder& encoder)
{
    // check for free encoders
    GPU::FenceID fence = GPU::FenceID::invalid();
    if(!free_fences.is_empty())
    {
        fence = free_fences.pop();
    }
    else
    {
        fence = GPU::fence_create(
            {
                .device = render_device->get_graphics_device(),
                .signaled = false,
            }
        );
        (void)work_fences.add(fence);
    }

    // submit encoder

    GPU::queue_execute_command_buffer(render_device->get_graphics_queue(),
        {
            .wait_semaphores = {},
            .wait_stages = {},
            .command_buffers = Slice<const GPU::CommandBufferID>(&encoder.command_buffer, 1),
            .signal_semaphores = {},
            .fence = fence,
        }
    );

    (void)work_submited.add(
        WorkSubmit
        {
            .fence = fence,
            .encoder = encoder,
        }
    );
}

void CommandQueue::_remove_finished_work()
{
    for(usize i = 0; i < work_submited.count; i++)
    {
        WorkSubmit& send_data = work_submited.get(i);
        if(GPU::fence_get_state(send_data.fence) == true)
        {
            GPU::fence_reset(Slice(&send_data.fence, 1));
            free_fences.push(send_data.fence);
            free_encoders.push(send_data.encoder);
            work_submited.remove_at(i);
            i--;
        }
    }
}
