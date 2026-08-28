#include "Platform/win32/win32_mutex.h"


static inline void _mutex_lock(SRWLOCK* srw)
{
    AcquireSRWLockExclusive(srw);
}

static inline bool _mutex_try_lock(SRWLOCK* srw)
{
    return TryAcquireSRWLockExclusive(srw);
}

static inline void _mutex_unlock(SRWLOCK* srw)
{
    ReleaseSRWLockExclusive(srw);
}

Mutex Mutex::create()
{
    Mutex mutex = {};
    SRWLOCK* srw = reinterpret_cast<SRWLOCK*>(&mutex.impl);
    *srw = SRWLOCK_INIT;
    return mutex;
}

void Mutex::destroy()
{

}

void Mutex::lock()
{
    _mutex_lock(reinterpret_cast<SRWLOCK*>(&impl));
}

bool Mutex::try_lock()
{
    return _mutex_try_lock(reinterpret_cast<SRWLOCK*>(&impl));
}

void Mutex::unlock()
{
    _mutex_unlock(reinterpret_cast<SRWLOCK*>(&impl));
}


