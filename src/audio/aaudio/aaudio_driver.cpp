#include "audio/aaudio/aaudio_driver.h"


InternalAudio::AudioAdapter AAudioDriver::get_vtable()
{
    return InternalAudio::AudioAdapter
    {
        .initialize = &AAudioDriver::initialize,
        .shutdown = &AAudioDriver::shutdown,
    };
}

void AAudioDriver::initialize(mem::Allocator* allocator)
{
    data.allocator = allocator;
}

void AAudioDriver::shutdown()
{

}
