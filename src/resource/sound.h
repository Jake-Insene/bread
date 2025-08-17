#pragma once
#include "audio/audio.h"
#include "resource/resource.h"

#include <external/dr_wav.h>


struct Sound : Resource
{
    RESOURCE(RESOURCE_SOUND, .LoadFromAssets = true, .Extensions = "wav");

    struct InternalData
    {
        Audio::SourceVoiceID source_voice;
    } data;

    void init();
    void destroy();

    void load(StringView file_path);

    [[nodiscard]] Audio::SourceVoiceID get_source_voice() const { return data.source_voice; }
};