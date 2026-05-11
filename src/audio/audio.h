#pragma once
#include "audio/audio_structs.h"
#include "mem/allocator.h"


namespace InternalAudio
{
struct AudioAdapter;
}

/*
* For the audio services and engine, try to load i16 pcm frames
* and convert them to device format at the end. Always feed the Audio driver with 2 channels.
*/
struct Audio
{
    static constexpr usize OutputChannels = 2;
    
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

    static void initialize(const mem::Allocator& allocator, DriverType driver);
    static void initialize_from_adapter(const InternalAudio::AudioAdapter* adapter);
    static void shutdown();
    
    static InternalAudio::AudioAdapter* get_adapter();

    static u32 output_get_samples_per_sec();

    static void output_start();
    static void output_stop();
    static bool output_wait_for_event();
    static u32 output_get_frame_count();
    static void output_send_frames(const Slice<i16>& frames);
};

