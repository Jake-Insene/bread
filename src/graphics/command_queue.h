#pragma once
#include "collections/array.h"
#include "collections/stack.h"
#include "gpu/gpu.h"
#include "mem/allocator.h"
#include "mem/stack_allocator.h"
#include "graphics/command_encoder.h"


namespace Graphics
{

struct CommandQueueInfo
{
    GPU::DeviceID device;
    GPU::QueueID queue;
};

struct CommandQueueExecuteInfo
{
    Slice<GPU::SemaphoreID> wait_semaphores;
    Slice<GPU::PipelineStages> wait_stages;
    Slice<GPU::SemaphoreID> signal_semaphores;
    const CommandEncoder& encoder;
};

struct CommandQueueExecuteEmptyInfo
{
    Slice<GPU::SemaphoreID> wait_semaphores;
    Slice<GPU::PipelineStages> wait_stages;
    Slice<GPU::SemaphoreID> signal_semaphores;
};

struct CommandQueue
{
    static constexpr usize TmpAllocatorSize = 1024 * 1024;

    mem::Allocator allocator;
    
    GPU::DeviceID device;
    GPU::QueueID queue;
    GPU::CommandPoolID command_pool;

    struct WorkSubmit
    {
        GPU::FenceID fence;
        CommandEncoder encoder;
        bool empty;
    };

    mem::StackAllocator tmp_allocator;
    Array<CommandEncoder> encoders;
    Array<GPU::FenceID> work_fences;
    Array<WorkSubmit> work_submited;
    
    Stack<GPU::FenceID> free_fences;
    Stack<CommandEncoder> free_encoders;

    void init(const mem::Allocator& _allocator, const CommandQueueInfo& info);
    void destroy();

    CommandEncoder acquire_encoder();
    GPU::FenceID execute(const CommandQueueExecuteInfo& info);
    GPU::FenceID execute_empty(const CommandQueueExecuteEmptyInfo& info);

    void wait_for_all();

    void release_fence(GPU::FenceID fence);

    void _remove_finished_work();
};

}
