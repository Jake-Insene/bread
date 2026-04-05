#include "audio/audio.h"

#include "debug/fail.h"


#if defined(BREAD_WIN32)
#include "audio/wasapi/wasapi_driver.h"
#endif


void Audio::initialize(const mem::Allocator& allocator, DriverType driver)
{
	switch (driver)
	{
	case Audio::DriverType::Wasapi:
#if defined(BREAD_WIN32)
		vtable = WASAPIDriver::get_vtable();
#endif
		break;
	default:
		FailOn(true, "unknown audio driver");
		break;
	}

#if defined(BREAD_WIN32)
	vtable.initialize(allocator);
#else
    Unused(allocator);
#endif
}
