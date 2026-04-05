#include "audio/audio.h"

#include "debug/fail.h"


#if defined(BREAD_WIN32)
#include "audio/wasapi/wasapi_driver.h"
#elif defined(BREAD_ANDROID)
#include "audio/aaudio/aaudio_driver.h"
#endif


static inline Audio::VTable current_adapter = {};

void Audio::initialize(const mem::Allocator& allocator, DriverType driver)
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

void Audio::initialize_from_adapter(const VTable *adapter)
{
	current_adapter = *adapter;
}

void Audio::shutdown()
{
	current_adapter.shutdown();
}

Audio::VTable* Audio::get_adapter()
{
	return &current_adapter;
}
