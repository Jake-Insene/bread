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
    };

    static void initialize(const mem::Allocator& allocator, DriverType driver);
    static void initialize_from_adapter(const VTable* adapter);
    static void shutdown();
    
    static VTable* get_adapter();

    static Audio::Format output_get_format();
    static u32 output_get_channels();
    static u32 output_get_samples_per_sec();
    static u32 output_get_bits_per_sample();
};

