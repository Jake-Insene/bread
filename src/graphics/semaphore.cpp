#include "graphics/semaphore.h"


namespace Graphics
{

void Semaphore::init(const mem::Allocator& _allocator, Device* _parent, GPU::DeviceID gpu_device)
{
    DeviceObject::init(_allocator, _parent);
    gpu_semaphore = GPU::semaphore_create(
        {
            .device = gpu_device,
        }
    );
}

void Semaphore::destroy()
{
    GPU::semaphore_destroy(gpu_semaphore);
    DeviceObject::destroy();
}

}