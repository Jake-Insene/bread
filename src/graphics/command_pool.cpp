#include "graphics/command_pool.h"


namespace Graphics
{

void CommandPool::init(const CommandPoolInfo& info)
{
    data.allocator = info.allocator;
    data.device = info.device;
    data.queue_usage = info.queue_usage;

    data.gpu_command_pool = GPU::command_pool_create(
        data.device,
        {
            .usage = data.queue_usage,
        }
    );

    data.command_buffers = Array<GPU::CommandBufferID>::with_size(data.allocator, 4);
    data.gpu_work_fences = Array<GPU::FenceID>::with_size(data.allocator, 4);
    data.work_submited = Array<WorkSubmit>::with_size(data.allocator, 4);

    data.gpu_free_fences = Stack<GPU::FenceID>::with_size(data.allocator, 4);
    data.free_command_buffers = Stack<GPU::CommandBufferID>::with_size(data.allocator, 4);
}

void CommandPool::destroy()
{
    for(GPU::CommandBufferID command_buffer : data.command_buffers.iter())
    {
        GPU::command_buffer_free(command_buffer);
    }

    for(GPU::FenceID fence : data.gpu_work_fences.iter())
    {
        GPU::fence_destroy(fence);
    }
    data.command_buffers.destroy();
    data.gpu_work_fences.destroy();
    data.work_submited.destroy();

    data.gpu_free_fences.destroy();
    data.free_command_buffers.destroy();

    GPU::command_pool_destroy(data.gpu_command_pool);
}

GPU::CommandBufferID CommandPool::acquire_command_buffer()
{
    if(!data.free_command_buffers.is_empty())
    {
        return data.free_command_buffers.pop();
    }

    GPU::CommandBufferID command_buffer = GPU::command_buffer_allocate(data.device, {.pool = data.gpu_command_pool});
    return data.command_buffers.add(command_buffer);
}

GPU::FenceID CommandPool::execute(GPU::QueueID queue, const CommandPoolExecuteInfo& info)
{
    // check for free command buffers
    GPU::FenceID fence = GPU::FenceID::invalid();
    if(!data.gpu_free_fences.is_empty())
    {
        fence = data.gpu_free_fences.pop();
        GPU::fence_reset(Slice(&fence, 1));
    }
    else
    {
        fence = _alloc_new_fence();
    }

    // submit command buffer
    GPU::queue_execute_command_buffer(queue,
        {
            .wait_semaphores = info.wait_semaphores,
            .wait_stages = info.wait_stages,
            .command_buffers = Slice(&info.command_buffer, 1),
            .signal_semaphores = info.signal_semaphores,
            .fence = fence,
        }
    );

    (void)data.work_submited.add(
        WorkSubmit
        {
            .fence = fence,
            .command_buffer = info.command_buffer,
            .empty = false,
        }
    );
    
    return fence;
}

GPU::FenceID CommandPool::execute_empty(GPU::QueueID queue, const CommandPoolExecuteEmptyInfo& info)
{
    // check for free command buffers
    GPU::FenceID fence = GPU::FenceID::invalid();
    if(!data.gpu_free_fences.is_empty())
    {
        fence = data.gpu_free_fences.pop();
        GPU::fence_reset(Slice(&fence, 1));
    }
    else
    {
        fence = _alloc_new_fence();
    }

    // submit
    GPU::queue_execute_command_buffer(queue,
        {
            .wait_semaphores = info.wait_semaphores,
            .wait_stages = info.wait_stages,
            .command_buffers = {},
            .signal_semaphores = info.signal_semaphores,
            .fence = fence,
        }
    );
    
    (void)data.work_submited.add(
        WorkSubmit
        {
            .fence = fence,
            .command_buffer = GPU::CommandBufferID::invalid(),
            .empty = true,
        }
    );

    return fence;
}

void CommandPool::wait_for_all()
{
    if(data.work_submited.count == 0)
    {
        return;
    }

    Slice fences = data.allocator->array<GPU::FenceID>(data.work_submited.count);
    for(usize i = 0; i < data.work_submited.count; i++)
    {
        fences[i] = data.work_submited.get(i).fence;
    }

    GPU::fence_wait_for(fences, true, MaxValue<u64>);
    data.allocator->free(Mem::to_bytes(fences));
    _remove_finished_work();
}

void CommandPool::release_fence(GPU::FenceID fence)
{
    for(usize i = 0; i < data.work_submited.count; i++)
    {
        WorkSubmit& work_data = data.work_submited.get(i);
        if(work_data.fence != fence)
        {
            continue;
        }

        GPU::fence_reset(Slice(&work_data.fence, 1));
        data.gpu_free_fences.push(work_data.fence);
        if(!work_data.empty)
        {
            data.free_command_buffers.push(work_data.command_buffer);
        }
        data.work_submited.remove_at(i);
        break;
    }
}

GPU::FenceID CommandPool::_alloc_new_fence()
{
    GPU::FenceID fence = GPU::fence_create(data.device, {.signaled = false});
    (void)data.gpu_work_fences.add(fence);
    return fence;
}

void CommandPool::_remove_finished_work()
{
    for(usize i = 0; i < data.work_submited.count; i++)
    {
        WorkSubmit& work_data = data.work_submited.get(i);
        if(!GPU::fence_get_state(work_data.fence))
        {
            continue;
        }

        data.gpu_free_fences.push(work_data.fence);
        if(!work_data.empty)
        {
            data.free_command_buffers.push(work_data.command_buffer);
        }
        data.work_submited.remove_at(i);
        i--;
    }
}

}
