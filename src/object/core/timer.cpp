#include "object/core/timer.h"


void Timer::init(const CreateInfo& info)
{
    Object::init(info);
    data.acumulator = 0;
}

void Timer::deinit()
{
    Object::deinit();
}

void Timer::update(f32 dt)
{
    if(!data.started)
        return;

    data.acumulator += dt;

    if(data.acumulator >= duration)
    {
        if(timeout.has_func())
            timeout.call(this);
     
        if(data.loop)
        {
            data.acumulator = 0;
        }
        else
        {
            data.started = false;
        }
    }
}


void Timer::start()
{
    data.acumulator = 0;
    data.started = true;
}

void Timer::stop()
{
    data.acumulator = 0;
    data.started = false;
}

void Timer::set_loop(bool enable)
{
    if (data.loop == enable)
        return;

    data.loop = enable;
}

