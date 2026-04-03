#pragma once
#include "gpu/gpu.h"


namespace Graphics
{

struct Semaphore
{
    GPU::SemaphoreID gpu_semaphore;

    void init(GPU::DeviceID gpu_device);
    void destroy();
};

}
