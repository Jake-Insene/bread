#pragma once
#include "core/header.h"


struct [[nodiscard]] Semaphore
{
    Opaque* impl;
    
    static Semaphore create(usize initial_value);

    void destroy();

    void signal();
    void wait();
};

