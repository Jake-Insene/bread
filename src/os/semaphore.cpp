#include "os/semaphore.h"


Semaphore Semaphore::create(usize initial_value)
{
    return Semaphore
    {
        .id = OS::semaphore_create(initial_value),
    };
}

void Semaphore::destroy()
{
    OS::semaphore_destroy(id);
}

void Semaphore::signal()
{
    OS::semaphore_signal(id);
}

void Semaphore::wait()
{
    OS::semaphore_wait(id);
}
