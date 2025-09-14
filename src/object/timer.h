#pragma once
#include "object/object.h"

/*
* A timer is a simple object that counts from a to b.
* when reaches b it calls the timeout event.
* You can start()/stop() of make it a loop.
*/
struct Timer : Object
{
    OBJECT(Timer, Object);

    f32 duration = 0;
    
    Event<void(Object::*)()> timeout;

    struct InternalData
    {
        f32 acumulator = 0;
        bool loop = false;
    } data;

    void init(const CreateInfo&);
    
    void enter();
    void internal_update(f32 dt);

    void start();
    void stop();

    void set_loop(bool enable);
    [[nodiscard]] bool get_loop() const { return data.loop; }
};
