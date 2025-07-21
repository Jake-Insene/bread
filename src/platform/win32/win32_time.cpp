#include "core/time.h"

#include "platform/platform_header.h"

f64 Time::get_time()
{
	LARGE_INTEGER platform_time;
	LARGE_INTEGER platform_frequency;
	QueryPerformanceFrequency(&platform_frequency);
	QueryPerformanceCounter(&platform_time);
	return (f64)platform_time.QuadPart / (f64)platform_frequency.QuadPart;
}
