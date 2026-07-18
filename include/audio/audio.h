#pragma once
#include "core/header.h"
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

    template<typename T>
    requires(IsArithmetic<T>)
    struct FrameT
    {
        using Type = T;

        Type left;
        Type right;

        constexpr FrameT() : left(0), right(0) {}
        constexpr FrameT(Type _left, Type _right) : left(_left), right(_right) {}

        constexpr void add(const FrameT& frame)
        {
            left += frame.left;
            right += frame.right;
        }

        template<typename MT>
        requires(IsArithmetic<MT>)
        constexpr void mul(const MT value)
        {
            left *= value;
            right *= value;
        }
    };

    using Frame = FrameT<i16>;
    using FrameF = FrameT<f32>;
    
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

    static void initialize(Mem::Allocator* allocator, DriverType driver);
    static void initialize_from_adapter(const InternalAudio::AudioAdapter* adapter);
    static void shutdown();
    
    static InternalAudio::AudioAdapter* get_adapter();

    static u32 output_get_samples_per_sec();

    static void output_start();
    static void output_stop();
    static bool output_wait_for_event();
    static u32 output_get_frame_count();
    static void output_send_frames(const Slice<Frame>& frames);
};

