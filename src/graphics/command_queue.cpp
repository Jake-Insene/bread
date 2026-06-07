#include "graphics/command_queue.h"

#include "graphics/device.h"
#include "graphics/fence.h"
#include "os/os.h"


namespace Graphics
{

void CommandQueue::init(Mem::Allocator* _allocator, Device* _parent, const CommandQueueInfo& info)
{
    DeviceObject::init(_allocator, _parent);
    gpu_device = info.gpu_device;
    gpu_queue = info.gpu_queue;
    gpu_command_pool = GPU::command_pool_create(
        {
            .device = info.gpu_device,
            .queue = info.gpu_queue,
        }
    );

    tmp_allocator.init(OS::map_memory(TmpAllocatorSize, OS::MapReadWrite));
    encoders = Array<CommandEncoder>::with_size(allocator, 4);
    gpu_work_fences = Array<Fence*>::with_size(allocator, 4);
    work_submited = Array<WorkSubmit>::with_size(allocator, 4);

    gpu_free_fences = Stack<Fence*>::with_size(allocator, 4);
    free_encoders = Stack<CommandEncoder>::with_size(allocator, 4);
}

void CommandQueue::destroy()
{
    GPU::queue_wait_idle(gpu_queue);

    for(CommandEncoder& encoder : encoders.iter())
    {
        GPU::command_buffer_free(encoder.command_buffer);
    }
    for(Fence* fence : gpu_work_fences.iter())
    {
        fence->destroy();
    }
    encoders.destroy();
    gpu_work_fences.destroy();
    work_submited.destroy();
    OS::unmap_memory(tmp_allocator.sp);

    gpu_free_fences.destroy();
    free_encoders.destroy();

    GPU::command_pool_destroy(gpu_command_pool);
    DeviceObject::destroy();
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
        .allocator = &tmp_allocator,
        .command_buffer = GPU::command_buffer_allocate(
            {
                .pool = gpu_command_pool,
            }
        )
    };

    return encoders.add(encoder);
}

Fence* CommandQueue::execute(const CommandQueueExecuteInfo& info)
{
    // check for free encoders
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

    // submit encoder
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

    GPU::queue_execute_command_buffer(gpu_queue,
        {
            .wait_semaphores = gpu_wait_semaphores,
            .wait_stages = info.wait_stages,
            .command_buffers = Slice<const GPU::CommandBufferID>(&info.encoder->command_buffer, 1),
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
            .encoder = *info.encoder,
            .empty = false,
        }
    );
    
    return fence;
}

Fence* CommandQueue::execute_empty(const CommandQueueExecuteEmptyInfo& info)
{
    // check for free encoders
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

    GPU::queue_execute_command_buffer(gpu_queue,
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

    Mem::Allocator* allocator = &tmp_allocator;

    Slice<GPU::FenceID> fences = allocator->array<GPU::FenceID>(work_submited.count);
    for(usize i = 0; i < work_submited.count; i++)
    {
        fences[i] = work_submited.get(i).fence->gpu_fence;
    }

    GPU::fence_wait_for(fences, true, MaxValue<u64>);
    _remove_finished_work();

    tmp_allocator.reset();
}

void CommandQueue::release_fence(Fence* fence)
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
            free_encoders.push(work_data.encoder);
        }
        work_submited.remove_at(i);
        break;
    }
}

Fence* CommandQueue::_alloc_new_fence()
{
    Fence* fence = parent->create_fence(false);
    (void)gpu_work_fences.add(fence);
    return fence;
}

void CommandQueue::_remove_finished_work()
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
            free_encoders.push(work_data.encoder);
        }
        work_submited.remove_at(i);
        i--;
    }
}

}
