#pragma once
#include "mem/allocator.h"
#include "gpu/gpu.h"


namespace Graphics
{

struct Queue
{
    Mem::Allocator* allocator;

    GPU::QueueID gpu_queue;

    void init(Mem::Allocator* _allocator, const GPU::QueueCreateInfo& info);
    void destroy();

    void wait_idle();
    GPU::AcquireResult present(const GPU::QueuePresentInfo& present_info);
};

}
