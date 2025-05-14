#include "objects/timer.h"


void Timer::start()
{
    mark_internal_update();
    data.acumulator = 0;
}

void Timer::internal_update(f64 delta)
{
    data.acumulator += delta;

    if(data.acumulator >= duration)
    {
        if(timeout.has_func())
            timeout.call();
     
        if(loop)
            data.acumulator = 0;
        else
            Object::data.flags.unset(Object::FLAG_INTERNAL_UPDATE);
    }
}