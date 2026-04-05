#include "audio/aaudio/aaudio_driver.h"


Audio::VTable AAudioDriver::get_vtable()
{
    return Audio::VTable
    {
        .initialize = &AAudioDriver::initialize,
        .shutdown = &AAudioDriver::shutdown,
    };
}

void AAudioDriver::initialize(const mem::Allocator& allocator)
{
    data.allocator = allocator;
}

void AAudioDriver::shutdown()
{

}
