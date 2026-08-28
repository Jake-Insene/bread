#pragma once
#include "Audio/Audio.hpp"


namespace InternalAudio
{

struct AudioAdapter
{
    virtual ~AudioAdapter() {}
    
    virtual u32 output_get_samples_per_sec() = 0;

    virtual void output_start() = 0;
    virtual void output_stop() = 0;
    virtual bool output_wait_for_event() = 0;
    virtual u32 output_get_frame_count() = 0;
    virtual void output_send_frames(const Slice<Audio::Frame>& frames) = 0;
};

}