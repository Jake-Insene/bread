#include "Platform/win32/win32_atomic.h"



u64 AtomicImpl::AtomicIncrement64(u64 volatile* memory)
{
    return static_cast<u64>(
        InterlockedIncrement64(reinterpret_cast<i64 volatile*>(memory))
    );
}

u64 AtomicImpl::AtomicExchange(u64 volatile* memory, u64 value)
{
    return static_cast<u64>(
        InterlockedExchange64(
            reinterpret_cast<i64 volatile*>(memory),
            static_cast<i64>(value)
        )
    );
}

u64 AtomicImpl::AtomicCompareExchange(u64 volatile* memory, u64 exchange, u64 compare)
{
    return static_cast<u64>(
        InterlockedCompareExchange64(
            reinterpret_cast<i64 volatile*>(memory),
            static_cast<i64>(exchange),
            static_cast<i64>(compare)
        )
    );
}
