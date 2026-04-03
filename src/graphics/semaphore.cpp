#include "graphics/semaphore.h"


namespace Graphics
{

void Semaphore::init(GPU::DeviceID gpu_device)
{
    gpu_semaphore = GPU::semaphore_create(
        {
            .device = gpu_device,
        }
    );
}

void Semaphore::destroy()
{
    GPU::semaphore_destroy(gpu_semaphore);
}

}