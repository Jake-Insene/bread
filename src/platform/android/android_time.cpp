#include "core/time.h"

#include "platform/platform_header.h"


void Time::initialize()
{}

void Time::shutdown()
{}

f64 Time::get_time()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}
