#include "Audio/Audio.hpp"

#include "Audio/AudioAdapter.hpp"
#include "Debug/Fail.hpp"


#if defined(BREAD_WIN32)
#include "Audio/WASAPI/WASAPIDriver.hpp"
#elif defined(BREAD_ANDROID)
#include "audio/aaudio/aaudio_driver.h"
#endif


static inline InternalAudio::AudioAdapter current_adapter = {};

void Audio::initialize(Mem::Allocator& allocator)
{
#if defined(BREAD_WIN32)
	current_adapter = WASAPIDriver::get_vtable();
#elif defined(BREAD_ANDROID)
	current_adapter = AAudioDriver::get_vtable();
#else
#error "Platform don't implemented"
#endif

	current_adapter.initialize(allocator);
}

void Audio::initialize_from_adapter(const InternalAudio::AudioAdapter *adapter)
{
	current_adapter = *adapter;
}

void Audio::shutdown()
{
	current_adapter.shutdown();
}

InternalAudio::AudioAdapter* Audio::get_adapter()
{
	return &current_adapter;
}

u32 Audio::output_get_samples_per_sec()
{
	return current_adapter.output_get_samples_per_sec();
}

void Audio::output_start()
{
	current_adapter.output_start();
}

void Audio::output_stop()
{
	current_adapter.output_stop();
}

bool Audio::output_wait_for_event()
{
	return current_adapter.output_wait_for_event();
}

u32 Audio::output_get_frame_count()
{
	return current_adapter.output_get_frame_count();
}

void Audio::output_send_frames(const Slice<Frame>& frames)
{
	current_adapter.output_send_frames(frames);
}


