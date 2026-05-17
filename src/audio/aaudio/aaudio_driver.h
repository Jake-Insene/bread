#pragma once
#include "audio/audio_adapter.h"



struct AAudioDriver
{
    struct InternalData
    {
        mem::Allocator* allocator;
    };

    static inline InternalData data = {};

    static InternalAudio::AudioAdapter get_vtable();

    static void initialize(mem::Allocator* allocator);
    static void shutdown();
};
