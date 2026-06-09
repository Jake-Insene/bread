#include "graphics/queue.h"

#include "graphics/command_buffer.h"
#include "graphics/fence.h"
#include "graphics/semaphore.h"


namespace Graphics
{

void Queue::init(Mem::Allocator* _allocator, GPU::DeviceID gpu_device, GPU::QueueUsage usage, u32 index)
{
    allocator = _allocator;

    gpu_queue = GPU::queue_get(
        {
            .device = gpu_device,
            .usage = usage,
            .index = index,
        }
    );
    gpu_queue_usage = usage;
}

void Queue::destroy()
{
}

void Queue::wait_idle()
{
    GPU::queue_wait_idle(gpu_queue);
}

void Queue::execute(const QueueExecuteInfo& execute_info)
{
    // submit command buffer
    Slice<GPU::SemaphoreID> gpu_wait_semaphores = allocator->array<GPU::SemaphoreID>(execute_info.wait_semaphores.len);
    Slice<GPU::CommandBufferID> gpu_command_buffers = allocator->array<GPU::CommandBufferID>(execute_info.command_buffers.len);
    Slice<GPU::SemaphoreID> gpu_signal_semaphores = allocator->array<GPU::SemaphoreID>(execute_info.signal_semaphores.len);
    for(usize i = 0; i < gpu_wait_semaphores.len; i++)
    {
        gpu_wait_semaphores[i] = execute_info.wait_semaphores[i]->gpu_semaphore;
    }
    for(usize i = 0; i < gpu_command_buffers.len; i++)
    {
        gpu_command_buffers[i] = execute_info.command_buffers[i]->gpu_command_buffer;
    }
    for(usize i = 0; i < gpu_signal_semaphores.len; i++)
    {
        gpu_signal_semaphores[i] = execute_info.signal_semaphores[i]->gpu_semaphore;
    }

    GPU::queue_execute_command_buffer(gpu_queue,
        {
            .wait_semaphores = gpu_wait_semaphores,
            .wait_stages = execute_info.wait_stages,
            .command_buffers = gpu_command_buffers,
            .signal_semaphores = gpu_signal_semaphores,
            .fence = execute_info.fence->gpu_fence,
        }
    );
    allocator->free(Mem::to_bytes(gpu_wait_semaphores));
    allocator->free(Mem::to_bytes(gpu_command_buffers));
    allocator->free(Mem::to_bytes(gpu_signal_semaphores));
}

GPU::AcquireResult Queue::present(const GPU::QueuePresentInfo& present_info)
{
    return GPU::queue_present(gpu_queue, present_info);
}

}
