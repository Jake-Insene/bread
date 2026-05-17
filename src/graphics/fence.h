#pragma once
#include "gpu/gpu.h"
#include "graphics/device_object.h"


namespace Graphics
{

struct Fence : DeviceObject
{
    GPU::FenceID gpu_fence;
    
    void init(mem::Allocator* _allocator, Device* _parent, GPU::DeviceID gpu_device, bool signaled);
    void destroy();

    void reset();
    void wait(u64 timeout);
    bool get_state();
};

}
