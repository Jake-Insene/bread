#pragma once
#include "Audio/AudioAdapter.hpp"
#include "Audio/Audio.hpp"
#include "Debug/Fail.hpp"
#include "Debug/Log.hpp"
#include "platform/platform_header.h"


#define WASAPIDebugInfo(...) Log::debug("[WASAPIDriver]: " __VA_ARGS__)
#define WASAPIDebugAssert(cond, ...) DebugAssert(cond, "[WASAPIDriver]: " __VA_ARGS__)
#define WASAPIFailOn(cond, ...) FailOn(cond, "[WASAPIDriver]: " __VA_ARGS__)
#define WASAPIFatal(...) Fatal("[WASAPIDriver]: " __VA_ARGS__)


struct WASAPIDriver final : InternalAudio::AudioAdapter
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
        Mem::Allocator& allocator;

        IMMDeviceEnumerator* enumerator;
        WASAPIDevice output_device;
        WASAPIDevice input_device;

        HANDLE event_handle;

        InternalData(Mem::Allocator& allocator)
        : allocator(allocator)
        {}
    };

    WASAPIDriver(Mem::Allocator& allocator);
    virtual ~WASAPIDriver() override;

    u32 output_get_samples_per_sec() override;

    void output_start() override;
    void output_stop() override;
    bool output_wait_for_event() override;
    u32 output_get_frame_count() override;
    void output_send_frames(const Slice<Audio::Frame>& frames) override;
};

