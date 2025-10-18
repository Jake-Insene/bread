#pragma once
#include "os/os.h"


struct [[nodiscard]] Semaphore
{
    static constexpr usize MaxValue = MaxValue<usize>;
    
    OS::SemaphoreID id;
    
    static Semaphore create(usize initial_value);

    void destroy();

    void signal();
    void wait();
};

struct [[nodiscard]] ScopedSemaphore
{
    Semaphore sem;

    ScopedSemaphore(usize value) : sem(Semaphore::create(value)) {}
    ~ScopedSemaphore() { sem.destroy(); }
};
