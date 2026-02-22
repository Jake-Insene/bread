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

		.source_voice_set_volume = &XAudio2Driver::source_voice_set_volume,
		.source_voice_get_volume = &XAudio2Driver::source_voice_get_volume,
		
		.source_voice_play = &XAudio2Driver::source_voice_play,
		.source_voice_stop = &XAudio2Driver::source_voice_stop,
		.source_voice_keep_playing = &XAudio2Driver::source_voice_keep_playing,
	};
}


void XAudio2Driver::initialize(const mem::Allocator& allocator)
{
	data.allocator = allocator;

	data.source_voices = FreeList<SourceVoice, Audio::SourceVoiceID>::with_size(
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
	data.xaudio->StartEngine();
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
	sv.wfx = wfx;

	sv.callback = get_allocator().object<VoiceCallback>();
	data.xaudio->CreateSourceVoice(&sv.sv_xaudio, &wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, sv.callback);
	DebugAssert(sv.sv_xaudio != nullptr, "couldn't create a source voice");

	sv.buffer = get_allocator().array<u8>(create_info.buffer.len);
	mem::copy(sv.buffer, create_info.buffer);

	return sv_id;
}

void XAudio2Driver::destroy_source_voice(Audio::SourceVoiceID sv_id)
{
	SourceVoice& sv = _get_source_voice(sv_id);
	sv.sv_xaudio->DestroyVoice();
	get_allocator().free(
		Slice(reinterpret_cast<u8*>(sv.callback), 1)
	);
	get_allocator().free(sv.buffer);

	data.source_voices.remove(sv_id);
}

void XAudio2Driver::source_voice_set_volume(Audio::SourceVoiceID sv_id, f32 volume)
{
	SourceVoice& sv = _get_source_voice(sv_id);
	sv.sv_xaudio->SetVolume(volume);
}

f32 XAudio2Driver::source_voice_get_volume(Audio::SourceVoiceID sv_id)
{
	SourceVoice& sv = _get_source_voice(sv_id);

	f32 volume;
	sv.sv_xaudio->GetVolume(&volume);
	return volume;
}

void XAudio2Driver::source_voice_play(Audio::SourceVoiceID sv_id)
{
	SourceVoice& sv = _get_source_voice(sv_id);
	sv.is_playing = true;

	XAUDIO2_BUFFER buffer = {};
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.AudioBytes = static_cast<UINT32>(sv.buffer.len);
	buffer.pAudioData = sv.buffer.ptr();

	sv.sv_xaudio->SubmitSourceBuffer(&buffer);
	sv.sv_xaudio->Start();
}

void XAudio2Driver::source_voice_stop(Audio::SourceVoiceID sv_id)
{
	SourceVoice& sv = _get_source_voice(sv_id);
	sv.is_playing = false;
	sv.sv_xaudio->Stop();
}

void XAudio2Driver::source_voice_keep_playing(Audio::SourceVoiceID sv_id)
{
	SourceVoice& sv = _get_source_voice(sv_id);

	if (sv.callback->state == VOICE_STATE_STREAM_END)
	{
		XAUDIO2_BUFFER buffer = {};
		buffer.Flags = XAUDIO2_END_OF_STREAM;
		buffer.AudioBytes = static_cast<UINT32>(sv.buffer.len);
		buffer.pAudioData = sv.buffer.ptr();

		sv.sv_xaudio->SubmitSourceBuffer(&buffer);
		sv.callback->state = VOICE_STATE_UNKNOWN;
	}
}

