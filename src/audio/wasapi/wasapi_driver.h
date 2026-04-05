#pragma once
#include "audio/audio.h"
#include "debug/debug.h"
#include "debug/fail.h"
#include "log/log.h"
#include "platform/platform_header.h"


#define WASAPIDebugInfo(...) Log::debug("[WASAPIDriver]: " __VA_ARGS__)
#define WASAPIDebugAssert(cond, ...) DebugAssert(cond, "[WASAPIDriver]: " __VA_ARGS__)
#define WASAPIFailOn(cond, ...) FailOn(cond, "[WASAPIDriver]: " __VA_ARGS__)
#define WASAPIFatal(...) Fatal("[WASAPIDriver]: " __VA_ARGS__)

struct WASAPIDriver
{
    struct WASAPIDevice
    {
        IMMDevice* device;
        IAudioClient* audio_client;
        IAudioRenderClient* render_client;
    };

    struct InternalData
    {
        mem::Allocator allocator;

        IMMDeviceEnumerator* enumerator;
        WASAPIDevice output_device;
        WASAPIDevice input_device;
    };

    static inline InternalData data;

    static Audio::VTable get_vtable();

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();
};

