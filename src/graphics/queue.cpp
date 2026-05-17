#include "graphics/queue.h"


namespace Graphics
{

void Queue::init(mem::Allocator* _allocator, const GPU::QueueCreateInfo& info)
{
    allocator = _allocator;

    gpu_queue = GPU::queue_create(info);
}

void Queue::destroy()
{
    GPU::queue_destroy(gpu_queue);
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
