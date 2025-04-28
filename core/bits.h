#pragma once
#include "core/types.h"
#include "core/macros.h"

template<usize bits = sizeof(usize)*8>
struct [[nodiscard]] BitField
{
    static_assert((bits & 0x3F) != bits, "Bits is not a multiple of 64");

    usize data[bits/64];
    
    constexpr void set(const usize index, const bool v)
    {
        DebugAssert(index < bits, "Invalid bit field");
        usize b = usize(v) << (index & 0x3F);
        data[(index / 64)] |= b;
    }
    constexpr void unset(const usize index)
    {
        DebugAssert(index < bits, "Invalid bit field");
        data[(index / 64)] &= ~(1 << (index & 0x3F));
    }

    [[nodiscard]] constexpr bool is_set(const usize index) const
    { 
        DebugAssert(index < bits, "Invalid bit field");
        return bool(data[(index / 64)] << (index & 0x3F));
    }
    
    constexpr void clear() { ::new(data) usize[bits/64]{}; }
};
