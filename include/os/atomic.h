#pragma once
#include "Core/Header.hpp"


struct AtomicImpl
{
    static u64 AtomicIncrement64(u64 volatile* memory);
    static u64 AtomicExchange(u64 volatile* memory, u64 value);

    static u64 AtomicCompareExchange(u64 volatile* memory, u64 exchange, u64 compare);
};

template<typename T>
struct alignas(16) Atomic
{
    static_assert(sizeof(T) <= 8, "only primitive types");

    u64 memory;

    static Atomic create()
    {
        return Atomic
        {
            .memory = static_cast<u64>(T()),
        };
    }

    T increment()
    {
        return static_cast<T>(AtomicImpl::AtomicIncrement64(reinterpret_cast<u64*>(&memory)));
    }

    T load()
    {
        return static_cast<T>(AtomicImpl::AtomicCompareExchange(reinterpret_cast<u64*>(&memory), 0, 0));
    }
};