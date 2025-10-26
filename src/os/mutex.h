#pragma once
#include "os/os.h"


struct [[nodiscard]] Mutex
{
    OS::MutexID id;

    static Mutex create();

    void destroy();

    void lock();
    bool try_lock();
    void unlock();
};

