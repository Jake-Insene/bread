#pragma once
#include "core/header.h"
#include "collections/slice.h"


// Only for XAudio2 for now
// https://learn.microsoft.com/en-us/windows/win32/api/mmeapi/ns-mmeapi-waveformatex
struct AudioWaveFormat
{
	u16 channel_number;
	u32 samples_per_sec;
	u32 avg_bytes_per_sec;
	u16 block_align;
	u16 bits_per_sample;
};

struct AudioSourceVoiceCreateInfo
{
	AudioWaveFormat awf;
	Slice<u8> buffer;
};