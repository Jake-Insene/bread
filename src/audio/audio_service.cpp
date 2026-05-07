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

        u32 frame_count = 0;
        f32* buffer = Audio::output_get_buffer(&frame_count)->cast<f32*>();

        for(usize i = 0; i < frame_count; i++)
        {
            // fill buffer
            buffer[(i *2) + 0] = 0.F;
            buffer[(i *2) + 1] = 0.F;
        }

        Audio::output_release_buffer(frame_count);
    }
}

void AudioService::initialize(const AudioServiceCreateInfo& info)
{
    data.allocator = info.allocator;
    Audio::output_start();

    data.request_destroy = Atomic<bool>::create();
    data.output_thread = Thread::create(&_audio_output_thread, Opaque::from(*this));
}

void AudioService::shutdown()
{
    data.request_destroy.increment();
    data.output_thread.destroy();

    Audio::output_stop();
}

void AudioService::tick()
{}
