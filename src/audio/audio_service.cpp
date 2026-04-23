#include "audio/audio_service.h"



void AudioService::initialize(const SystemInitializeInfo& info)
{
    allocator = info.allocator;
}

void AudioService::shutdown()
{

}
