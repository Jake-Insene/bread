#pragma once
#include "core/types.h"


template<typename T>
inline constexpr usize __string_len(const T* str)
{
    usize len = 0;
    while (*str++)
    {
        len++;
    }

    return len;
}