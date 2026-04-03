#pragma once
#include "gpu/gpu.h"


namespace Graphics
{

struct Fence
{
    GPU::FenceID gpu_fence;
    
    void init(GPU::DeviceID gpu_device, bool signaled);
    void destroy();

    void reset();
    void wait(u64 timeout);
    bool get_state();
};

}
