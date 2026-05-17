#pragma once
#include "audio/audio.h"


namespace InternalAudio
{

struct AudioAdapter
{
    VTFunc(void, initialize, mem::Allocator*);
    VTFunc(void, shutdown);
    
    VTFunc(u32, output_get_samples_per_sec);

    VTFunc(void, output_start);
    VTFunc(void, output_stop);
    VTFunc(bool, output_wait_for_event);
    VTFunc(u32, output_get_frame_count, );
    VTFunc(void, output_send_frames, const Slice<Audio::Frame>& frames);
};

}