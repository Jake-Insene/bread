#pragma once
#include "gpu/gpu.h"
#include "graphics/device_object.h"


namespace Graphics
{

struct Semaphore : DeviceObject
{
    GPU::SemaphoreID gpu_semaphore;

    void init(mem::Allocator* _allocator, Device* _parent, GPU::DeviceID gpu_device);
    void destroy();
};

}
