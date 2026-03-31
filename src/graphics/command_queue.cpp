#include "graphics/command_queue.h"

#include "os/os.h"


namespace Graphics
{

void CommandQueue::init(const mem::Allocator& _allocator, const CommandQueueInfo& info)
{
    allocator = _allocator;

    device = info.device;
    queue = info.queue;
    command_pool = GPU::command_pool_create(
        {
            .device = info.device,
            .queue = info.queue,
        }
    );

    tmp_allocator.init(OS::map_memory(TmpAllocatorSize, OS::MapReadWrite));
    encoders = Array<CommandEncoder>::with_size(allocator, 4);
    work_fences = Array<GPU::FenceID>::with_size(allocator, 4);
    work_submited = Array<WorkSubmit>::with_size(allocator, 4);

    free_fences = Stack<GPU::FenceID>::with_size(allocator, 4);
    free_encoders = Stack<CommandEncoder>::with_size(allocator, 4);
}

void CommandQueue::destroy()
{
    GPU::queue_wait_idle(queue);

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
    OS::unmap_memory(tmp_allocator.sp);

    free_fences.destroy();
    free_encoders.destroy();

    GPU::command_pool_destroy(command_pool);
}

CommandEncoder CommandQueue::acquire_encoder()
{
    if(!free_encoders.is_empty())
    {
        return free_encoders.pop();
    }

    tmp_allocator.reset();
    CommandEncoder encoder =
    {
        .allocator = tmp_allocator.allocator(),
        .command_buffer = GPU::command_buffer_allocate(
            {
                .pool = command_pool,
            }
        )
    };

    return encoders.add(encoder);
}

GPU::FenceID CommandQueue::execute(const CommandQueueExecuteInfo& info)
{
    // check for free encoders
    GPU::FenceID fence = GPU::FenceID::invalid();
    if(!free_fences.is_empty())
    {
        fence = free_fences.pop();
        GPU::fence_reset(Slice(&fence, 1));
    }
    else
    {
        fence = GPU::fence_create(
            {
                .device = device,
                .signaled = false,
            }
        );
        (void)work_fences.add(fence);
    }

    // submit encoder

    GPU::queue_execute_command_buffer(queue,
        {
            .wait_semaphores = info.wait_semaphores,
            .wait_stages = info.wait_stages,
            .command_buffers = Slice<const GPU::CommandBufferID>(&info.encoder.command_buffer, 1),
            .signal_semaphores = info.signal_semaphores,
            .fence = fence,
        }
    );

    (void)work_submited.add(
        WorkSubmit
        {
            .fence = fence,
            .encoder = info.encoder,
            .empty = false,
        }
    );
    
    return fence;
}

GPU::FenceID CommandQueue::execute_empty(const CommandQueueExecuteEmptyInfo& info)
{
    // check for free encoders
    GPU::FenceID fence = GPU::FenceID::invalid();
    if(!free_fences.is_empty())
    {
        fence = free_fences.pop();
        GPU::fence_reset(Slice(&fence, 1));
    }
    else
    {
        fence = GPU::fence_create(
            {
                .device = device,
                .signaled = false,
            }
        );
        (void)work_fences.add(fence);
    }

    GPU::queue_execute_command_buffer(queue,
        {
            .wait_semaphores = info.wait_semaphores,
            .wait_stages = info.wait_stages,
            .command_buffers = {},
            .signal_semaphores = info.signal_semaphores,
            .fence = fence,
        }
    );
    
    (void)work_submited.add(
        WorkSubmit
        {
            .fence = fence,
            .encoder = {},
            .empty = true,
        }
    );

    return fence;
}

void CommandQueue::wait_for_all()
{
    if(work_submited.count == 0)
    {
        return;
    }

    mem::Allocator allocator = tmp_allocator.allocator();

    Slice<GPU::FenceID> fences = allocator.array<GPU::FenceID>(work_submited.count);
    for(usize i = 0; i < work_submited.count; i++)
    {
        fences[i] = work_submited.get(i).fence;
    }

    GPU::fence_wait_for(fences, true, MaxValue<u64>);
    _remove_finished_work();

    tmp_allocator.reset();
}

void CommandQueue::release_fence(GPU::FenceID fence)
{
    for(usize i = 0; i < work_submited.count; i++)
    {
        WorkSubmit& work_data = work_submited.get(i);
        if(work_data.fence != fence)
        {
            continue;
        }

        GPU::fence_reset(Slice(&work_data.fence, 1));
        free_fences.push(work_data.fence);
        if(work_data.empty == false)
        {
            free_encoders.push(work_data.encoder);
        }
        work_submited.remove_at(i);
        break;
    }
}

void CommandQueue::_remove_finished_work()
{
    for(usize i = 0; i < work_submited.count; i++)
    {
        WorkSubmit& work_data = work_submited.get(i);
        if(GPU::fence_get_state(work_data.fence) == false)
        {
            continue;
        }

        free_fences.push(work_data.fence);
        if(work_data.empty == false)
        {
            free_encoders.push(work_data.encoder);
        }
        work_submited.remove_at(i);
        i--;
    }
}

}
