#pragma once
#include "core/header.h"


#define OSMutexAuto(mutex_ref) \
    [[maybe_unused]] MutexAutoLock __auto_mutex##__LINE__ = MutexAutoLock(mutex_ref);


struct [[nodiscard]] Mutex
{
    Opaque* impl;

    static Mutex create();

    void destroy();

    void lock();
    bool try_lock();
    void unlock();
};

struct MutexAutoLock
{
    Mutex* mutex;

    MutexAutoLock(Mutex* _mutex) : mutex(_mutex) { mutex->lock(); }
    ~MutexAutoLock(){ mutex->unlock(); }
};
