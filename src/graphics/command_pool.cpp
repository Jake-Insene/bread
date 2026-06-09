#include "graphics/command_pool.h"

#include "graphics/device.h"
#include "graphics/fence.h"
#include "graphics/semaphore.h"


namespace Graphics
{

void CommandPool::init(Mem::Allocator* _allocator, Device* _parent, const CommandPoolInfo& info)
{
    DeviceObject::init(_allocator, _parent);
    gpu_device = info.gpu_device;
    gpu_queue_usage = info.gpu_queue_usage;
    gpu_command_pool = GPU::command_pool_create(
        {
            .device = info.gpu_device,
            .usage = info.gpu_queue_usage,
        }
    );

    command_buffers = Array<CommandBuffer*>::with_size(allocator, 4);
    gpu_work_fences = Array<Fence*>::with_size(allocator, 4);
    work_submited = Array<WorkSubmit>::with_size(allocator, 4);

    gpu_free_fences = Stack<Fence*>::with_size(allocator, 4);
    free_command_buffers = Stack<CommandBuffer*>::with_size(allocator, 4);
}

void CommandPool::destroy()
{
    for(CommandBuffer* command_buffer : command_buffers.iter())
    {
        command_buffer->destroy();
    }

    for(Fence* fence : gpu_work_fences.iter())
    {
        fence->destroy();
    }
    command_buffers.destroy();
    gpu_work_fences.destroy();
    work_submited.destroy();

    gpu_free_fences.destroy();
    free_command_buffers.destroy();

    GPU::command_pool_destroy(gpu_command_pool);
    DeviceObject::destroy();
}

CommandBuffer* CommandPool::acquire_command_buffer()
{
    if(!free_command_buffers.is_empty())
    {
        return free_command_buffers.pop();
    }

    CommandBuffer* command_buffer = parent->_allocate_object<CommandBuffer>();
    command_buffer->init(allocator, parent, this);

    return command_buffers.add(command_buffer);
}

Fence* CommandPool::execute(Queue* queue, const CommandPoolExecuteInfo& info)
{
    // check for free command buffers
    Fence* fence = nullptr;
    if(!gpu_free_fences.is_empty())
    {
        fence = gpu_free_fences.pop();
        fence->reset();
    }
    else
    {
        fence = _alloc_new_fence();
    }

    // submit command buffer
    Slice<GPU::SemaphoreID> gpu_wait_semaphores = allocator->array<GPU::SemaphoreID>(info.wait_semaphores.len);
    Slice<GPU::SemaphoreID> gpu_signal_semaphores = allocator->array<GPU::SemaphoreID>(info.signal_semaphores.len);
    for(usize i = 0; i < gpu_wait_semaphores.len; i++)
    {
        gpu_wait_semaphores[i] = info.wait_semaphores[i]->gpu_semaphore;
    }
    for(usize i = 0; i < gpu_signal_semaphores.len; i++)
    {
        gpu_signal_semaphores[i] = info.signal_semaphores[i]->gpu_semaphore;
    }

    GPU::queue_execute_command_buffer(queue->gpu_queue,
        {
            .wait_semaphores = gpu_wait_semaphores,
            .wait_stages = info.wait_stages,
            .command_buffers = Slice<const GPU::CommandBufferID>(&info.command_buffer->gpu_command_buffer, 1),
            .signal_semaphores = gpu_signal_semaphores,
            .fence = fence->gpu_fence,
        }
    );
    allocator->free(Mem::to_bytes(gpu_wait_semaphores));
    allocator->free(Mem::to_bytes(gpu_signal_semaphores));

    (void)work_submited.add(
        WorkSubmit
        {
            .fence = fence,
            .command_buffer = info.command_buffer,
            .empty = false,
        }
    );
    
    return fence;
}

Fence* CommandPool::execute_empty(Queue* queue, const CommandPoolExecuteEmptyInfo& info)
{
    // check for free command buffers
    Fence* fence = nullptr;
    if(!gpu_free_fences.is_empty())
    {
        fence = gpu_free_fences.pop();
        fence->reset();
    }
    else
    {
        fence = _alloc_new_fence();
    }

    // submit
    Slice<GPU::SemaphoreID> gpu_wait_semaphores = allocator->array<GPU::SemaphoreID>(info.wait_semaphores.len);
    Slice<GPU::SemaphoreID> gpu_signal_semaphores = allocator->array<GPU::SemaphoreID>(info.signal_semaphores.len);
    for(usize i = 0; i < gpu_wait_semaphores.len; i++)
    {
        gpu_wait_semaphores[i] = info.wait_semaphores[i]->gpu_semaphore;
    }
    for(usize i = 0; i < gpu_signal_semaphores.len; i++)
    {
        gpu_signal_semaphores[i] = info.signal_semaphores[i]->gpu_semaphore;
    }

    GPU::queue_execute_command_buffer(queue->gpu_queue,
        {
            .wait_semaphores = gpu_wait_semaphores,
            .wait_stages = info.wait_stages,
            .command_buffers = {},
            .signal_semaphores = gpu_signal_semaphores,
            .fence = fence->gpu_fence,
        }
    );
    allocator->free(Mem::to_bytes(gpu_wait_semaphores));
    allocator->free(Mem::to_bytes(gpu_signal_semaphores));
    
    (void)work_submited.add(
        WorkSubmit
        {
            .fence = fence,
            .command_buffer = nullptr,
            .empty = true,
        }
    );

    return fence;
}

void CommandPool::wait_for_all()
{
    if(work_submited.count == 0)
    {
        return;
    }

    Slice<GPU::FenceID> fences = allocator->array<GPU::FenceID>(work_submited.count);
    for(usize i = 0; i < work_submited.count; i++)
    {
        fences[i] = work_submited.get(i).fence->gpu_fence;
    }

    GPU::fence_wait_for(fences, true, MaxValue<u64>);
    _remove_finished_work();
}

void CommandPool::release_fence(Fence* fence)
{
    for(usize i = 0; i < work_submited.count; i++)
    {
        WorkSubmit& work_data = work_submited.get(i);
        if(work_data.fence != fence)
        {
            continue;
        }

        work_data.fence->reset();
        gpu_free_fences.push(work_data.fence);
        if(!work_data.empty)
        {
            free_command_buffers.push(work_data.command_buffer);
        }
        work_submited.remove_at(i);
        break;
    }
}

Fence* CommandPool::_alloc_new_fence()
{
    Fence* fence = parent->create_fence(false);
    (void)gpu_work_fences.add(fence);
    return fence;
}

void CommandPool::_remove_finished_work()
{
    for(usize i = 0; i < work_submited.count; i++)
    {
        WorkSubmit& work_data = work_submited.get(i);
        if(!work_data.fence->get_state())
        {
            continue;
        }

        gpu_free_fences.push(work_data.fence);
        if(!work_data.empty)
        {
            free_command_buffers.push(work_data.command_buffer);
        }
        work_submited.remove_at(i);
        i--;
    }
}

}
