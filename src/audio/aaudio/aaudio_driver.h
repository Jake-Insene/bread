#pragma once
#include "audio/audio.h"


struct AAudioDriver
{
    struct InternalData
    {
        mem::Allocator allocator;
    };

    static inline InternalData data = {};

    static Audio::VTable get_vtable();

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();
};
