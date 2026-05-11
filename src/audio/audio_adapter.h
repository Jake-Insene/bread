#pragma once
#include "audio/audio.h"


namespace InternalAudio
{

struct AudioAdapter
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

}