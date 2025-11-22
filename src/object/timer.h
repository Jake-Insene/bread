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

    struct InternalData
    {
        f32 acumulator = 0;
        bool loop = false;
    } data;

    /*
    * The timer duration in seconds.
    */
    f32 duration = 0;
    /*
    * Timeout event, it's called when the timer reachs duration.
    */
    Event<void(Object::*)()> timeout;

    void enter();
    void internal_update(f32 dt);

    /*
    * Start the timer and resets it to 0.
    */
    void start();

    /*
    * Stop the timer and resets it to 0.
    */
    void stop();

    /*
    * @param enable True makes the timer restart after timeout otherwise it's stoped.
    */
    void set_loop(bool enable);

    /*
    * @return If the timer restart after timeout.
    */
    [[nodiscard]] bool get_loop() const { return data.loop; }
};
