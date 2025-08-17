#pragma once
#include "audio/audio_structs.h"
#include "mem/allocator.h"


struct Audio
{
    enum DriverType
    {
        DRIVER_UNKNOWN = 0,

        XAUDIO2,

        DEFAULT_DRIVER = XAUDIO2,
    };

    using SourceVoiceID = ID<i32>;

    struct VTable
    {
        VTFunc(void, initialize, const mem::Allocator&);
        VTFunc(void, shutdown);

        VTFunc(SourceVoiceID, create_source_voice, const AudioSourceVoiceCreateInfo&);
        VTFunc(void, destroy_source_voice, SourceVoiceID);

        VTFunc(void, source_voice_play, SourceVoiceID);
    };

    static inline VTable vtable;

    static void initialize(const mem::Allocator& allocator, DriverType driver);
    VTFuncDefS(shutdown);

    VTFuncDefArg1RetS(SourceVoiceID, create_source_voice, const AudioSourceVoiceCreateInfo&);
    VTFuncDefArg1S(destroy_source_voice, SourceVoiceID);

    VTFuncDefArg1S(source_voice_play, SourceVoiceID);

};

