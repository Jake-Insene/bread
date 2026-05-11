#include "audio/audio_service.h"

#include "audio/audio.h"
#include "math/funcs.h"


void _audio_output_thread(Opaque* self)
{
    AudioService* audio_service = self->cast<AudioService*>();

    while(true)
    {
        bool destroy_requested = audio_service->data.request_destroy.load();
        if(destroy_requested)
        {
            return;
        }

        bool waited = Audio::output_wait_for_event();
        if(!waited)
        {
            continue;
        }

        u32 frame_count = Audio::output_get_frame_count();
        Slice<i16> samples = audio_service->data.output_buffer.slice(frame_count * Audio::OutputChannels);

        for(usize i = 0; i < frame_count; i++)
        {
            // fill buffer
            samples[(i * Audio::OutputChannels) + 0] = 0;
            samples[(i * Audio::OutputChannels) + 1] = 0;
        }

        Audio::output_send_frames(samples);
    }
}

void AudioService::initialize(const AudioServiceCreateInfo& info)
{
    data.allocator = info.allocator;
    Audio::output_start();

    data.request_destroy = Atomic<bool>::create();

    data.output_buffer = data.allocator.array<i16>(Audio::output_get_samples_per_sec() * Audio::OutputChannels);
    data.output_thread = Thread::create(&_audio_output_thread, Opaque::from(*this));
}

void AudioService::shutdown()
{
    data.request_destroy.increment();
    data.output_thread.destroy();
    data.allocator.free(mem::to_bytes(data.output_buffer));

    Audio::output_stop();
}

void AudioService::tick()
{}
