#pragma once
#include "audio/audio.h"
#include "collections/queue_array.h"
#include "platform/platform_header.h"

#define XAudio2Fatal(...) Fatal("[XAudio2Driver]: " __VA_ARGS__)
#define XAudio2DebugInfo(...) DebugInfo("[XAudio2Driver]: " __VA_ARGS__)


struct XAudio2Driver
{
	struct SourceVoice
	{
		IXAudio2SourceVoice* sv_xaudio;
		Slice<u8> buffer;
		bool is_playing;
	};

	struct InternalData
	{
		mem::Allocator allocator;

		QueueArray<SourceVoice, Audio::SourceVoiceID> source_voices;

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
	static void source_voice_play(Audio::SourceVoiceID sv_id);
};