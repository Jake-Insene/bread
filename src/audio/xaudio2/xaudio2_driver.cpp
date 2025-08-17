#include "audio/xaudio2/xaudio2_driver.h"


#define HRCheckResult(x, ...) if(FAILED(x)) { XAudio2Fatal(__VA_ARGS__); }


Audio::VTable XAudio2Driver::get_vtable()
{
	return Audio::VTable
	{
		.initialize = &XAudio2Driver::initialize,
		.shutdown = &XAudio2Driver::shutdown,

		.create_source_voice = &XAudio2Driver::create_source_voice,
		.destroy_source_voice = &XAudio2Driver::destroy_source_voice,
		
		.source_voice_play = &XAudio2Driver::source_voice_play,
	};
}


void XAudio2Driver::initialize(const mem::Allocator& allocator)
{
	data.allocator = allocator;

	data.source_voices = QueueArray<SourceVoice, Audio::SourceVoiceID>::with_size(
		data.allocator, 4
	);
	
	XAudio2DebugInfo("Initializing audio...");

	HRCheckResult(
		XAudio2Create(&data.xaudio, 0, XAUDIO2_DEFAULT_PROCESSOR),
		"Couldn't create the xaudio2 object interface"
	);

	HRCheckResult(
		data.xaudio->CreateMasteringVoice(&data.master_voice),
		"Couldn't create the master voice"
	);
}

void XAudio2Driver::shutdown()
{
	data.master_voice->DestroyVoice();
	data.xaudio->Release();

	data.source_voices.destroy();
}

Audio::SourceVoiceID XAudio2Driver::create_source_voice(const AudioSourceVoiceCreateInfo& create_info)
{
	WAVEFORMATEX wfx =
	{
		.wFormatTag = WAVE_FORMAT_PCM,
		.nChannels = create_info.awf.channel_number,
		.nSamplesPerSec = create_info.awf.samples_per_sec,
		.nAvgBytesPerSec = create_info.awf.avg_bytes_per_sec,
		.nBlockAlign = create_info.awf.block_align,
		.wBitsPerSample = create_info.awf.bits_per_sample,
		.cbSize = 0,
	};


	Audio::SourceVoiceID sv_id = data.source_voices.add(SourceVoice());
	SourceVoice& sv = _get_source_voice(sv_id);

	data.xaudio->CreateSourceVoice(&sv.sv_xaudio, &wfx);
	DebugAssert(sv.sv_xaudio != nullptr, "Couldn't create a source voice");

	sv.buffer = get_allocator().array<u8>(create_info.buffer.len);
	mem::copy(sv.buffer, create_info.buffer);

	return sv_id;
}

void XAudio2Driver::destroy_source_voice(Audio::SourceVoiceID sv_id)
{
	SourceVoice& sv = _get_source_voice(sv_id);
	sv.sv_xaudio->DestroyVoice();
	get_allocator().free(sv.buffer);

	data.source_voices.remove(sv_id);
}

void XAudio2Driver::source_voice_play(Audio::SourceVoiceID sv_id)
{
	SourceVoice& sv = _get_source_voice(sv_id);
	sv.is_playing = true;

	XAUDIO2_BUFFER buffer =
	{
		.Flags = XAUDIO2_END_OF_STREAM,
		.AudioBytes = (UINT32)sv.buffer.len,
		.pAudioData = sv.buffer.ptr(),
	};

	sv.sv_xaudio->SubmitSourceBuffer(&buffer);
	sv.sv_xaudio->Start(0);
}

