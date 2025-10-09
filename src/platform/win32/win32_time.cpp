#include "core/time.h"

#include "platform/platform_header.h"


static f64 _program_start;
static i64 _frequency;


void Time::initialize()
{
	LARGE_INTEGER platform_time;
	QueryPerformanceFrequency((LARGE_INTEGER*)&_frequency);
	QueryPerformanceCounter(&platform_time);

	_program_start = f64(platform_time.QuadPart) / f64(_frequency);
}

void Time::shutdown()
{

}

f64 Time::get_time()
{
	LARGE_INTEGER platform_time;
	QueryPerformanceCounter(&platform_time);
	return (f64(platform_time.QuadPart) / f64(_frequency)) - _program_start;
}
