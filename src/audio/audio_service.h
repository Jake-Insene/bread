#pragma once
#include "audio/audio.h"
#include "collections/array.h"
#include "collections/string.h"
#include "mem/allocator.h"
#include "os/atomic.h"
#include "os/mutex.h"
#include "os/thread.h"


struct Sound;

struct AudioServiceCreateInfo
{
    mem::Allocator* allocator;
};

struct AudioService
{
    struct PlayInfo
    {
        f32 volume;
        bool loop;
    };

    struct EnqueuePlay
    {
        Sound* sound;
        PlayInfo play_info;

        usize frame_index;
    };

    struct Mixer
    {
        String name;
        f32 volume;

        Array<EnqueuePlay> plays;
    };

    struct InternalData
    {
        mem::Allocator* allocator;
        
        Mutex enqueue_mutex;
        Array<Mixer> mixers;
        
        Atomic<bool> request_destroy;
        
        Slice<Audio::Frame> output_buffer;
        Thread output_thread;
    } data;

    void initialize(const AudioServiceCreateInfo& info);
    void shutdown();

    u32 mixer_create(StringView mixer_name);
    u32 mixer_get_by_name(StringView mixer_name);
    u32 mixer_count();
    
    f32 mixer_get_volume(u32 mixer);
    void mixer_set_volume(u32 mixer, f32 new_volume);

    void mixer_play(u32 mixer, Sound* sound, const PlayInfo& play_info);

    void _mixer_mix(Mixer* mixer, Audio::FrameF* frame);
};
