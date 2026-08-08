#pragma once
#include "Core/Templates.h"


namespace Core
{

template<typename T>
requires(IsPrimitive<T>)
constexpr usize NullTerminatedLen(const T* str)
{
    usize len = 0;
    while (*str++)
    {
        len++;
    }

    return len;
}


template<typename T>
constexpr usize KiB(T n)
{
    return n * 1024;
}

template<typename T>
constexpr usize MiB(T n)
{
    return KiB(n) * 1024;
}

}
