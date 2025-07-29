#include "object/timer.h"


void Timer::init(const CreateInfo&)
{
    mark(MARK_INTERNAL_UPDATE);
}

void Timer::enter()
{
    data.acumulator = 0;
}

void Timer::internal_update(f64 delta)
{
    data.acumulator += delta;

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
    if(data.loop == false)
    {
        mark(MARK_INTERNAL_UPDATE);
    }
    data.acumulator = 0;
}

void Timer::stop()
{
    unmark(MARK_INTERNAL_UPDATE);
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
