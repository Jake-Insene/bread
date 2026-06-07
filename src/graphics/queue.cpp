#include "graphics/queue.h"


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
}

void Queue::destroy()
{
}

void Queue::wait_idle()
{
    GPU::queue_wait_idle(gpu_queue);
}

GPU::AcquireResult Queue::present(const GPU::QueuePresentInfo& present_info)
{
    return GPU::queue_present(gpu_queue, present_info);
}

}
