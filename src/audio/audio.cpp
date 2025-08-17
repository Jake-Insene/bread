#include "audio/audio.h"

#if defined(ENGINE_WIN32)
#include "audio/xaudio2/xaudio2_driver.h"
#endif


void Audio::initialize(const mem::Allocator& allocator, DriverType driver)
{
	switch (driver)
	{
	case Audio::XAUDIO2:
#if defined(ENGINE_WIN32)
		vtable = XAudio2Driver::get_vtable();
#endif
		break;
	default:
		FailOn(true, "Invalid graphics driver");
		break;
	}

	vtable.initialize(allocator);

}
