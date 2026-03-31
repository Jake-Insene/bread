#pragma once
#include "mem/allocator.h"
#include "gpu/gpu.h"


namespace Graphics
{

struct Queue
{
    mem::Allocator allocator;

    GPU::QueueID gpu_queue;

    void init(const mem::Allocator& _allocator, const GPU::QueueCreateInfo& info);
    void destroy();

    void wait_idle();
    GPU::AcquireResult present(const GPU::QueuePresentInfo& present_info);
};

}
