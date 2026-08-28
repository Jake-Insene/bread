#pragma once
#include "Core/Header.hpp"


struct [[nodiscard]] Semaphore
{
    Core::Opaque* impl;
    
    static Semaphore create(usize initial_value);

    void destroy();

    void signal();
    void wait();
};

