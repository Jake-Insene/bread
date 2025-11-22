#include "object/timer.h"


void Timer::enter()
{
    data.acumulator = 0;
}

void Timer::internal_update(f32 dt)
{
    data.acumulator += dt;

    if(data.acumulator >= duration)
    {
        if(timeout.has_func())
            timeout.call();
     
        if(data.loop)
            data.acumulator = 0;
        else
            unmark(MARK_INTERNAL_UPDATE);
    }
}


void Timer::start()
{
    mark(MARK_INTERNAL_UPDATE);
    data.acumulator = 0;
}

void Timer::stop()
{
    unmark(MARK_INTERNAL_UPDATE);
    data.acumulator = 0;
}

void Timer::set_loop(bool enable)
{
    if (data.loop == enable)
        return;

    data.loop = enable;

    if (enable)
        mark(MARK_INTERNAL_UPDATE);
    else
        unmark(MARK_INTERNAL_UPDATE);
}
