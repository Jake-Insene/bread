#include "object/audio_player.h"

#include "audio/audio.h"


void AudioPlayer::init(const CreateInfo&)
{
    mark(MARK_INTERNAL_UPDATE);
}

void AudioPlayer::enter()
{
}

void AudioPlayer::internal_update(f64 delta)
{
}


void AudioPlayer::play()
{
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
