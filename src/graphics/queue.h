#pragma once
#include "gpu/gpu.h"
#include "graphics/device_object.h"


namespace Graphics
{

struct QueueExecuteInfo
{
    Slice<Graphics::Semaphore*> wait_semaphores;
    Slice<const GPU::PipelineStages> wait_stages;
    Slice<const CommandBuffer*> command_buffers;
    Slice<Graphics::Semaphore*> signal_semaphores;
    Fence* fence;
};

struct Queue
{
    Mem::Allocator* allocator;

    GPU::QueueID gpu_queue;
    GPU::QueueUsage gpu_queue_usage;

    void init(Mem::Allocator* _allocator, GPU::DeviceID gpu_device, GPU::QueueUsage usage, u32 index);
    void destroy();

    void execute(const QueueExecuteInfo& execute_info);
    GPU::AcquireResult present(const GPU::QueuePresentInfo& present_info);

    void wait_idle();
};

}
