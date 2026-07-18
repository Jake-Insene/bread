#pragma once
#include "audio/audio_adapter.h"
#include "audio/audio.h"
#include "debug/debug.h"
#include "debug/fail.h"
#include "debug/log.h"
#include "platform/platform_header.h"


#define AAudioDebugInfo(...) Log::debug("[WASAPIDriver]: " __VA_ARGS__)
#define AAudioDebugAssert(cond, ...) DebugAssert(cond, "[WASAPIDriver]: " __VA_ARGS__)
#define AAudioFailOn(cond, ...) FailOn(cond, "[WASAPIDriver]: " __VA_ARGS__)
#define AAudioFatal(...) Fatal("[WASAPIDriver]: " __VA_ARGS__)


struct AAudioDriver
{
    struct InternalData
    {
        Mem::Allocator* allocator;
    };

    static inline InternalData data = {};

    static InternalAudio::AudioAdapter get_vtable();

    static void initialize(Mem::Allocator* allocator);
    static void shutdown();
};
