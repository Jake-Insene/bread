#pragma once
#include "audio/audio_structs.h"
#include "mem/allocator.h"


struct Audio
{
    enum Format
    {
        Unknown,
        PCM,
        IEEEFloat,
    };

    enum class DriverType
    {
        Unknown = 0,

        Wasapi,
        AAudio,

#if defined(BREAD_WIN32)
        Default = Wasapi,
#elif defined(BREAD_ANDROID)
        Default = AAudio,
#endif
    };

    struct VTable
    {
        VTFunc(void, initialize, const mem::Allocator&);
        VTFunc(void, shutdown);

        VTFunc(Audio::Format, output_get_format);
        VTFunc(u32, output_get_channels);
        VTFunc(u32, output_get_samples_per_sec);
        VTFunc(u32, output_get_bits_per_sample);

        VTFunc(void, output_start);
        VTFunc(void, output_stop);
        VTFunc(bool, output_wait_for_event);
        VTFunc(Opaque*, output_get_buffer, u32* out_frame_count);
        VTFunc(void, output_release_buffer, u32 frame_count);
    };

    static void initialize(const mem::Allocator& allocator, DriverType driver);
    static void initialize_from_adapter(const VTable* adapter);
    static void shutdown();
    
    static VTable* get_adapter();

    static Audio::Format output_get_format();
    static u32 output_get_channels();
    static u32 output_get_samples_per_sec();
    static u32 output_get_bits_per_sample();

    static void output_start();
    static void output_stop();
    static bool output_wait_for_event();
    static Opaque* output_get_buffer(u32* out_frame_count);
    static void output_release_buffer(u32 frame_count);
};

