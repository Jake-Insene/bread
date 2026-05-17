#include "graphics/fence.h"


namespace Graphics
{

void Fence::init(mem::Allocator* _allocator, Device* _parent, GPU::DeviceID gpu_device, bool signaled)
{
    DeviceObject::init(_allocator, _parent);
    gpu_fence = GPU::fence_create(
        {
            .device = gpu_device,
            .signaled = signaled,
        }
    );
}

void Fence::destroy()
{
    GPU::fence_destroy(gpu_fence);
    DeviceObject::destroy();
}

void Fence::reset()
{
    GPU::fence_reset(
        Slice(&gpu_fence, 1)
    );
}

void Fence::wait(u64 timeout)
{
    GPU::fence_wait_for(Slice(&gpu_fence, 1), true, timeout);
}

bool Fence::get_state()
{
    return GPU::fence_get_state(gpu_fence);
}

}
