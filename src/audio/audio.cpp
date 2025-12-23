#include "audio/audio.h"

#include "debug/fail.h"


#if defined(BREAD_WIN32)
#include "audio/xaudio2/xaudio2_driver.h"
#endif


void Audio::initialize(const mem::Allocator& allocator, DriverType driver)
{
	switch (driver)
	{
	case Audio::XAUDIO2:
#if defined(BREAD_WIN32)
		vtable = XAudio2Driver::get_vtable();
#endif
		break;
	default:
		FailOn(true, "Invalid graphics driver");
		break;
	}

#if !defined(BREAD_ANDROID)
	vtable.initialize(allocator);
#endif
}
