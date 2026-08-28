#include "audio/aaudio/aaudio_driver.h"


InternalAudio::AudioAdapter AAudioDriver::get_vtable()
{
    return InternalAudio::AudioAdapter
    {
        .initialize = &AAudioDriver::initialize,
        .shutdown = &AAudioDriver::shutdown,
    };
}

void AAudioDriver::initialize(Mem::Allocator& allocator)
{
    AAudioDebugInfo("Initializing WASAPI Driver...");

    // Ensures constructors are call.
    ConstructObject(data);

    data.allocator = allocator;
}

void AAudioDriver::shutdown()
{

}
