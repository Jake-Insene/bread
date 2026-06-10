#pragma once
#include "collections/array.h"
#include "collections/stack.h"
#include "gpu/gpu.h"


namespace Graphics
{

struct CommandPoolInfo
{
    Mem::Allocator* allocator;
    GPU::DeviceID device;
    GPU::QueueUsage queue_usage;
};

struct CommandPoolExecuteInfo
{
    Slice<const GPU::SemaphoreID> wait_semaphores;
    Slice<const GPU::PipelineStages> wait_stages;
    GPU::CommandBufferID command_buffer;
    Slice<const GPU::SemaphoreID> signal_semaphores;
};

struct CommandPoolExecuteEmptyInfo
{
    Slice<const GPU::SemaphoreID> wait_semaphores;
    Slice<const GPU::PipelineStages> wait_stages;
    Slice<const GPU::SemaphoreID> signal_semaphores;
};

struct CommandPool
{
    struct WorkSubmit
    {
        GPU::FenceID fence;
        GPU::CommandBufferID command_buffer;
        bool empty;
    };

    Mem::Allocator* allocator;
    GPU::DeviceID device;
    GPU::QueueUsage queue_usage;
    GPU::CommandPoolID gpu_command_pool;
    Array<GPU::CommandBufferID> command_buffers;
    Array<GPU::FenceID> gpu_work_fences;
    Array<WorkSubmit> work_submited;
    
    Stack<GPU::FenceID> gpu_free_fences;
    Stack<GPU::CommandBufferID> free_command_buffers;

    void init(const CommandPoolInfo& info);
    void destroy();

    GPU::CommandBufferID acquire_command_buffer();
    GPU::FenceID execute(GPU::QueueID queue, const CommandPoolExecuteInfo& info);
    GPU::FenceID execute_empty(GPU::QueueID queue, const CommandPoolExecuteEmptyInfo& info);

    void wait_for_all();

    void release_fence(GPU::FenceID fence);

    GPU::FenceID _alloc_new_fence();
    void _remove_finished_work();
};

}
