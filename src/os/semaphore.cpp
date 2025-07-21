#include "os/semaphore.h"

#include "platform/platform_header.h"


Semaphore Semaphore::create(usize initial_value)
{
    return Semaphore
    {
        .value = initial_value,
        .mutex = Mutex::create(),
    };
}

void Semaphore::destroy()
{
    mutex.destroy();
}

void Semaphore::signal()
{
    mutex.lock();
    {
        if (value >= MaxValue)
        {
            mutex.unlock();
            return;
        }
        value++;
    }
    mutex.unlock();
}

void Semaphore::wait()
{
    mutex.lock();
    usize cpy_value = value;
    mutex.unlock();
    while (cpy_value == 0)
    {
        mutex.lock();
        cpy_value = value;
        mutex.unlock();
    }
}
