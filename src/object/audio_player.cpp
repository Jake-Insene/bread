#include "object/audio_player.h"

#include "audio/audio.h"


void AudioPlayer::enter()
{
}

void AudioPlayer::exit()
{
    Audio::source_voice_stop(get_sound()->get_source_voice());
}

void AudioPlayer::internal_update(f32)
{
    if (get_sound() == nullptr)
        return;

    if (data.loop == false)
        return;
 
    Audio::source_voice_keep_playing(get_sound()->get_source_voice());
}


void AudioPlayer::play()
{
    if (get_sound() == nullptr)
        return;

    Audio::source_voice_play(get_sound()->get_source_voice());
    data.playing = true;
}

void AudioPlayer::stop()
{
    data.playing = false;
}

void AudioPlayer::set_loop(bool enable)
{
    if (data.loop == enable)
        return;

    data.loop = enable;
}

void AudioPlayer::set_volume(f32 new_volume)
{
    if (data.sound == nullptr)
        return;

    if (data.volume_cache == new_volume)
        return;

    Audio::source_voice_set_volume(get_sound()->get_source_voice(), new_volume);
    data.volume_cache = new_volume;
}
