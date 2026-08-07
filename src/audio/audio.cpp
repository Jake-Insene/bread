#include "audio/audio.h"

#include "audio/audio_adapter.h"
#include "debug/fail.h"


#if defined(BREAD_WIN32)
#include "audio/wasapi/wasapi_driver.h"
#elif defined(BREAD_ANDROID)
#include "audio/aaudio/aaudio_driver.h"
#endif


static inline InternalAudio::AudioAdapter current_adapter = {};

void Audio::initialize(Mem::Allocator& allocator, DriverType driver)
{
	switch (driver)
	{
	case Audio::DriverType::Wasapi:
#if defined(BREAD_WIN32)
		current_adapter = WASAPIDriver::get_vtable();
#endif
		break;
	case Audio::DriverType::AAudio:
#if defined(BREAD_ANDROID)
		current_adapter = AAudioDriver::get_vtable();
#endif
		break;
	default:
		FailOn(true, "unknown audio driver");
		break;
	}

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


