#pragma once
#include "core/types.h"


template<typename T>
constexpr usize __string_len(const T* str)
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

