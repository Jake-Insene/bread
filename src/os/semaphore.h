#pragma once
#include "os/os.h"


struct [[nodiscard]] Semaphore
{
    OS::SemaphoreID id;
    
    static Semaphore create(usize initial_value);

    void destroy();

    void signal();
    void wait();
};

