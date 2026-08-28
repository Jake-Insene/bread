#include "Platform/android/android_mutex.h"


Mutex Mutex::create()
{
    Mutex mutex = {};
    return mutex;
}

void Mutex::destroy()
{
}

void Mutex::lock()
{
}

bool Mutex::try_lock()
{
    return false;
}

void Mutex::unlock()
{
}


