#pragma once
#include "Core/Header.h"


struct [[nodiscard]] Semaphore
{
    Core::Opaque* impl;
    
    static Semaphore create(usize initial_value);

    void destroy();

    void signal();
    void wait();
};

