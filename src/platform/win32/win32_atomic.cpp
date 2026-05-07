#include "platform/win32/win32_atomic.h"



u64 AtomicImpl::AtomicIncrement64(u64 volatile* memory)
{
    return InterlockedIncrement(memory);
}

u64 AtomicImpl::AtomicExchange(u64 volatile* memory, u64 value)
{
    return InterlockedExchange(memory, value);
}

u64 AtomicImpl::AtomicCompareExchange(u64 volatile* memory, u64 exchange, u64 compare)
{
    return InterlockedCompareExchange64(
        reinterpret_cast<i64 volatile*>(memory),
        static_cast<i64>(exchange),
        static_cast<i64>(compare)
    );
}
