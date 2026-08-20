#pragma once
#include "Core/Header.hpp"
#include "Mem/Allocator.hpp"


namespace InternalAudio
{
struct AudioAdapter;
}

/**
* For the audio services and engine, submit normalized floating point values [-1, 1].
* Always feed the Audio driver with 2 channels.
*/
struct Audio
{
    static constexpr usize OutputChannels = 2;

    template<typename T>
    requires(Core::IsArithmetic<T>)
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
        requires(Core::IsArithmetic<MT>)
        constexpr void mul(const MT value)
        {
            left *= value;
            right *= value;
        }
    };

    using Frame = FrameT<f32>;
    
    static void initialize(Mem::Allocator& allocator);
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

