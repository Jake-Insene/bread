#pragma once
#include "core/types.h"
#include "debug/assertion.h"

template<usize bits = sizeof(usize)*8>
struct [[nodiscard]] BitField
{
    usize data[(bits >> 6) + 1];
    
    constexpr void set(const usize index)
    {
        DebugAssert(index < bits, "invalid bit field");
        usize b = 1ULL << (index & 0x3F);
        data[(index / 64)] |= b;
    }

    constexpr void unset(const usize index)
    {
        DebugAssert(index < bits, "invalid bit field");
        data[(index / 64)] &= ~(1 << (index & 0x3F));
    }

    [[nodiscard]] constexpr bool is_set(const usize index) const
    { 
        DebugAssert(index < bits, "invalid bit field");
        return bool(
            data[(index / 64)] & (1ULL << (index & 0x3F))
        );
    }
    
    constexpr void clear() { ::new(data) usize[bits/64]{}; }
};
