#include "audio/audio_service.h"

#include "audio/audio.h"
#include "math/funcs.h"
#include "resource/sound.h"


static inline void _audio_output_thread(Opaque* self)
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
        Slice<Audio::Frame> samples = audio_service->data.output_buffer.slice(frame_count);

        // getting enqueue plays
        audio_service->data.enqueue_mutex.lock();
        Slice<AudioService::Mixer> mixers = audio_service->data.mixers.slice();

        for(usize frame_i = 0; frame_i < frame_count; frame_i++)
        {
            Audio::FrameF frame_f = Audio::FrameF();
            for(AudioService::Mixer& mixer : mixers)
            {
                audio_service->_mixer_mix(&mixer, &frame_f);
            }
            
            Audio::Frame final_frame = Audio::Frame(
                i16(Math::clamp<f32>(frame_f.left, -32768.0F, 32767.0F)),
                i16(Math::clamp<f32>(frame_f.right, -32768.0F, 32767.0F))
            );

            samples[frame_i] = final_frame;
        }

        audio_service->data.enqueue_mutex.unlock();
        Audio::output_send_frames(samples);
    }
}

void AudioService::initialize(const AudioServiceCreateInfo& info)
{
    data.allocator = info.allocator;
    Audio::output_start();

    data.enqueue_mutex = Mutex::create();
    data.mixers = Array<Mixer>::with_size(data.allocator, 4);
    mixer_create("Master");

    data.request_destroy = Atomic<bool>::create();

    data.output_buffer = data.allocator->array<Audio::Frame>(Audio::output_get_samples_per_sec());
    data.output_thread = Thread::create(&_audio_output_thread, Opaque::from(*this));
}

void AudioService::shutdown()
{
    data.request_destroy.increment();
    data.output_thread.destroy();
    data.allocator->free(Mem::to_bytes(data.output_buffer));
    data.enqueue_mutex.destroy();

    (void)data.mixers.iter().for_each([](Mixer& mixer)
    {
        mixer.name.destroy();
        mixer.plays.destroy();
    });
    data.mixers.destroy();
    Audio::output_stop();
}

u32 AudioService::mixer_create(StringView mixer_name)
{
    OSMutexAuto(&data.enqueue_mutex);
    Mixer new_mixer =
    {
        .name = String::from_chars(data.allocator, mixer_name),
        .volume = 1.F,
        .plays = Array<EnqueuePlay>::with_size(data.allocator, 4),
    };

    (void)data.mixers.add(new_mixer);
    return data.mixers.count - 1;
}

u32 AudioService::mixer_get_by_name(StringView mixer_name)
{
    OSMutexAuto(&data.enqueue_mutex);
    for(u32 i = 0; i < data.mixers.count; i++)
    {
        Mixer& mix = data.mixers.get(i);
        if(mix.name.equals(mixer_name))
        {
            return i;
        }
    }

    return MaxValue<u32>;
}

u32 AudioService::mixer_count()
{
    OSMutexAuto(&data.enqueue_mutex);
    return data.mixers.count;
}

f32 AudioService::mixer_get_volume(u32 mixer)
{
    OSMutexAuto(&data.enqueue_mutex);
    return data.mixers.get(mixer).volume;
}

void AudioService::mixer_set_volume(u32 mixer, f32 new_volume)
{
    OSMutexAuto(&data.enqueue_mutex);
    data.mixers.get(mixer).volume = new_volume;
}

void AudioService::mixer_play(u32 mixer, Sound* sound, const PlayInfo& play_info)
{
    OSMutexAuto(&data.enqueue_mutex);
    Mixer& mix = data.mixers.get(mixer);

    (void)mix.plays.add(
        EnqueuePlay
        {
            .sound = sound,
            .play_info = play_info,
            .frame_index = 0,
        }
    );
}

void AudioService::_mixer_mix(Mixer* mixer, Audio::FrameF* frame)
{
    if(mixer->plays.count == 0)
    {
        return;
    }

    f32 normalize = 1 / f32(mixer->plays.count);
    for(usize play_i = 0; play_i < mixer->plays.count; play_i++)
    {
        AudioService::EnqueuePlay& enqueue_play = mixer->plays.get(play_i);

        if(enqueue_play.frame_index >= enqueue_play.sound->get_frame_count())
        {
            if(enqueue_play.play_info.loop)
            {
                enqueue_play.frame_index = 0;
            }
            else
            {
                // remove
                mixer->plays.remove_at(play_i);
                play_i--;
                continue;
            }
        }

        const Audio::Frame sample = enqueue_play.sound->get_frame(enqueue_play.frame_index);
        Audio::FrameF sample_f = Audio::FrameF(
            sample.left,
            sample.right
        );

        sample_f.mul(mixer->volume * enqueue_play.play_info.volume * normalize);
        frame->add(sample_f);

        enqueue_play.frame_index += 1;
    }
}
