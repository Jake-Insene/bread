#pragma once
#include "audio/audio.h"
#include "collections/free_list.h"
#include "platform/platform_header.h"

#define XAudio2Fatal(...) Fatal("[XAudio2Driver]: " __VA_ARGS__)
#define XAudio2DebugInfo(...) DebugInfo("[XAudio2Driver]: " __VA_ARGS__)


#include <atomic>

struct XAudio2Driver
{
	enum VoiceState
	{
		VOICE_STATE_UNKNOWN,
		VOICE_STATE_STREAM_END,
	};

	struct VoiceCallback : IXAudio2VoiceCallback
	{
		VoiceState state;

		void __stdcall OnVoiceProcessingPassStart(UINT32) override {}
		void __stdcall OnVoiceProcessingPassEnd() override {}
		void __stdcall OnStreamEnd() override { state = VOICE_STATE_STREAM_END; }
		void __stdcall OnBufferStart(void*) override {}
		void __stdcall OnBufferEnd(void*) override {}
		void __stdcall OnLoopEnd(void*) override {}
		void __stdcall OnVoiceError(void*, HRESULT) override {}
	};

	struct SourceVoice
	{
		VoiceCallback* callback;
		IXAudio2SourceVoice* sv_xaudio;
		WAVEFORMATEX wfx;

		Slice<u8> buffer;
		bool is_playing;
	};

	struct InternalData
	{
		mem::Allocator allocator;

		FreeList<SourceVoice, Audio::SourceVoiceID> source_voices;

		IXAudio2* xaudio;
		IXAudio2MasteringVoice* master_voice;
	};

	static inline InternalData data;

	static Audio::VTable get_vtable();

	[[nodiscard]] static mem::Allocator& get_allocator() { return data.allocator; }

	[[nodiscard]] static SourceVoice& _get_source_voice(Audio::SourceVoiceID sv_id) 
	{ 
		return data.source_voices.get(sv_id); 
	}

	static void initialize(const mem::Allocator& allocator);
	static void shutdown();

	static Audio::SourceVoiceID create_source_voice(const AudioSourceVoiceCreateInfo& create_info);
	static void destroy_source_voice(Audio::SourceVoiceID sv_id);
	
	static void source_voice_set_volume(Audio::SourceVoiceID sv_id, f32 volume);
	static f32 source_voice_get_volume(Audio::SourceVoiceID sv_id);

	static void source_voice_play(Audio::SourceVoiceID sv_id);
	static void source_voice_keep_playing(Audio::SourceVoiceID sv_id);
};