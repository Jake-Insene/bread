#pragma once
#include "collections/array.h"
#include "collections/stack.h"
#include "gpu/gpu.h"
#include "mem/allocator.h"
#include "mem/stack_allocator.h"
#include "graphics/command_encoder.h"
#include "graphics/semaphore.h"


namespace Graphics
{

struct CommandQueueInfo
{
    GPU::DeviceID gpu_device;
    GPU::QueueID gpu_queue;
};

struct CommandQueueExecuteInfo
{
    Slice<Ptr<Graphics::Semaphore>> wait_semaphores;
    Slice<GPU::PipelineStages> wait_stages;
    Slice<Ptr<Graphics::Semaphore>> signal_semaphores;
    const CommandEncoder& encoder;
};

struct CommandQueueExecuteEmptyInfo
{
    Slice<Ptr<Graphics::Semaphore>> wait_semaphores;
    Slice<GPU::PipelineStages> wait_stages;
    Slice<Ptr<Graphics::Semaphore>> signal_semaphores;
};

struct Fence;

struct CommandQueue
{
    static constexpr usize TmpAllocatorSize = 1024 * 1024;

    mem::Allocator allocator;
    
    GPU::DeviceID gpu_device;
    GPU::QueueID gpu_queue;
    GPU::CommandPoolID gpu_command_pool;

    struct WorkSubmit
    {
        Fence* fence;
        CommandEncoder encoder;
        bool empty;
    };

    mem::StackAllocator tmp_allocator;
    Array<CommandEncoder> encoders;
    Array<Fence*> gpu_work_fences;
    Array<WorkSubmit> work_submited;
    
    Stack<Fence*> gpu_free_fences;
    Stack<CommandEncoder> free_encoders;

    void init(const mem::Allocator& _allocator, const CommandQueueInfo& info);
    void destroy();

    CommandEncoder acquire_encoder();
    Fence* execute(const CommandQueueExecuteInfo& info);
    Fence* execute_empty(const CommandQueueExecuteEmptyInfo& info);

    void wait_for_all();

    void release_fence(Fence* fence);

    void _remove_finished_work();
};

}
