#include "platform/android/android_semaphore.h"


Semaphore Semaphore::create(usize)
{
    Semaphore semaphore = {};
    return semaphore;
}

void Semaphore::destroy()
{
}

void Semaphore::signal()
{
}

void Semaphore::wait()
{
}

