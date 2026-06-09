#pragma once
#include "collections/array.h"
#include "collections/stack.h"
#include "gpu/gpu.h"
#include "graphics/command_buffer.h"
#include "graphics/device_object.h"
#include "graphics/queue.h"


namespace Graphics
{

struct CommandPoolInfo
{
    GPU::DeviceID gpu_device;
    GPU::QueueUsage gpu_queue_usage;
};

struct CommandPoolExecuteInfo
{
    Slice<Graphics::Semaphore*> wait_semaphores;
    Slice<const GPU::PipelineStages> wait_stages;
    CommandBuffer* command_buffer;
    Slice<Graphics::Semaphore*> signal_semaphores;
};

struct CommandPoolExecuteEmptyInfo
{
    Slice<Graphics::Semaphore*> wait_semaphores;
    Slice<const GPU::PipelineStages> wait_stages;
    Slice<Graphics::Semaphore*> signal_semaphores;
};

struct CommandPool : DeviceObject
{
    GPU::DeviceID gpu_device;
    GPU::QueueUsage gpu_queue_usage;
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

    void init(Mem::Allocator* _allocator, Device* _parent, const CommandPoolInfo& info);
    void destroy();

    CommandBuffer* acquire_command_buffer();
    Fence* execute(Queue* queue, const CommandPoolExecuteInfo& info);
    Fence* execute_empty(Queue* queue, const CommandPoolExecuteEmptyInfo& info);

    void wait_for_all();

    void release_fence(Fence* fence);

    Fence* _alloc_new_fence();
    void _remove_finished_work();
};

}
