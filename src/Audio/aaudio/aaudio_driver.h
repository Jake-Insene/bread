#pragma once
#include "Audio/AudioAdapter.hpp"
#include "Audio/Audio.hpp"
#include "Debug/Assertion.hpp"
#include "Debug/Fail.hpp"
#include "Debug/Log.hpp"
#include "Platform/platform_header.h"


#define AAudioDebugInfo(...) Log::debug("[AAudioDriver]: " __VA_ARGS__)
#define AAudioDebugAssert(cond, ...) DebugAssert(cond, "[AAudioDriver]: " __VA_ARGS__)
#define AAudioFailOn(cond, ...) FailOn(cond, "[AAudioDriver]: " __VA_ARGS__)
#define AAudioFatal(...) Fatal("[AAudioDriver]: " __VA_ARGS__)


struct AAudioDriver
{
    struct InternalData
    {
        Mem::Allocator& allocator;
    };

    static inline InternalData data = {};

    static InternalAudio::AudioAdapter get_vtable();

    static void initialize(Mem::Allocator& allocator);
    static void shutdown();
};
