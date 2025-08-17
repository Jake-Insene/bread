#pragma once
#include "math/values.h"
#include "os/mutex.h"


struct [[nodiscard]] Semaphore
{
    static constexpr usize MaxValue = math::MaxValue<usize>;

    usize value;
    Mutex mutex;

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
