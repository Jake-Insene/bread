#pragma once
#include "core/header.h"

struct [[nodiscard]] Mutex
{
    using MutexID = ID<u32>;

    MutexID id;

    static Mutex create();

    void destroy();

    void lock();
    bool try_lock();
    void unlock();
};

struct ScopedMutex
{
    Mutex mutex;

    ScopedMutex()
        : mutex(Mutex::create())
    {
        mutex.lock();
    }

    ~ScopedMutex()
    {
        mutex.unlock();
        mutex.destroy();
    }
};
