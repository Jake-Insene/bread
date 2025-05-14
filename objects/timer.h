#pragma once
#include "objects/object.h"


struct Timer : Object
{
    OBJECT(Timer, Object);

    // Timer duration in seconds, when it reaches 0 the timeout event is called,
    // if loop is true the timer will reset and start again.
    f64 duration = 0;
    bool loop = false;
    
    Event<void(Object::*)()> timeout;

    // As everything in a struct is public we need to hide data
    // that should not be modified/access directly, this also
    // resolve some namespace problems.
    struct InternalData
    {
        f64 acumulator = 0;
    } data;

    void start();

    void internal_update(f64 dt);

};
