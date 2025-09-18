#include "os/mutex.h"


Mutex Mutex::create()
{
    return Mutex
    {
        .id = OS::mutex_create(),
    };
}

void Mutex::destroy()
{
    OS::mutex_destroy(id);
}

void Mutex::lock()
{
    OS::mutex_lock(id);
}

bool Mutex::try_lock()
{
    return OS::mutex_try_lock(id);
}

void Mutex::unlock()
{
    OS::mutex_unlock(id);
}

