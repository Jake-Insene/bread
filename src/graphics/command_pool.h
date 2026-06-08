#pragma once
#include "collections/array.h"
#include "collections/stack.h"
#include "gpu/gpu.h"
#include "mem/allocator.h"
#include "graphics/command_buffer.h"
#include "graphics/device_object.h"
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
    Slice<Graphics::Semaphore*> wait_semaphores;
    Slice<const GPU::PipelineStages> wait_stages;
    Slice<Graphics::Semaphore*> signal_semaphores;
    CommandBuffer* command_buffer;
};

struct CommandQueueExecuteEmptyInfo
{
    Slice<Graphics::Semaphore*> wait_semaphores;
    Slice<const GPU::PipelineStages> wait_stages;
    Slice<Graphics::Semaphore*> signal_semaphores;
};

struct Fence;

struct CommandPool : DeviceObject
{
    static constexpr usize TmpAllocatorSize = 1024 * 1024;
    
    GPU::DeviceID gpu_device;
    GPU::QueueID gpu_queue;
    GPU::CommandPoolID gpu_command_pool;

    struct WorkSubmit
    {
        Fence* fence;
        CommandBuffer* command_buffer;
        bool empty;
    };

    Array<CommandBuffer*> command_buffers;
    Array<Fence*> gpu_work_fences;
    Array<WorkSubmit> work_submited;
    
    Stack<Fence*> gpu_free_fences;
    Stack<CommandBuffer*> free_command_buffers;

    void init(Mem::Allocator* _allocator, Device* _parent, const CommandQueueInfo& info);
    void destroy();

    CommandBuffer* acquire_command_buffer();
    Fence* execute(const CommandQueueExecuteInfo& info);
    Fence* execute_empty(const CommandQueueExecuteEmptyInfo& info);

    void wait_for_all();

    void release_fence(Fence* fence);

    Fence* _alloc_new_fence();
    void _remove_finished_work();
};

}
