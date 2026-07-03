#pragma once
#include "audio/audio_adapter.h"
#include "audio/audio.h"
#include "debug/debug.h"
#include "debug/fail.h"
#include "debug/log.h"
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

        u32 wave_format;
        u32 channels;
        u32 samples_per_sec;
        u32 bits_per_sample;

        u32 frame_count;
    };

    struct InternalData
    {
        Mem::Allocator* allocator;

        IMMDeviceEnumerator* enumerator;
        WASAPIDevice output_device;
        WASAPIDevice input_device;

        HANDLE event_handle;
    };

    static inline InternalData data;

    static InternalAudio::AudioAdapter get_vtable();

    static void initialize(Mem::Allocator* allocator);
    static void shutdown();

    static u32 output_get_samples_per_sec();

    static void output_start();
    static void output_stop();
    static bool output_wait_for_event();

    static u32 output_get_frame_count();
    static void output_send_frames(const Slice<Audio::Frame>& frames);
};

