#pragma once
#include "Audio/Audio.hpp"


namespace InternalAudio
{

struct AudioAdapter
{
    void(*initialize)(Mem::Allocator&);
    void(*shutdown)();
    
    u32(*output_get_samples_per_sec)();

    void(*output_start)();
    void(*output_stop)();
    bool(*output_wait_for_event)();
    u32(*output_get_frame_count)();
    void(*output_send_frames)(const Slice<Audio::Frame>& frames);
};

}