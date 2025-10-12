#pragma once
#include "core/templates.h"
#include "debug/assertion.h"


/*
* Set/unset a collection of bits.
* Useful for flags.
*/
template<usize RequestedBits = sizeof(usize)*8>
struct [[nodiscard]] BitMask
{
    static constexpr usize get_required_len()
    {
        if constexpr(RequestedBits <= 64)
        {
            return 1;
        }
        else
        {
            return (RequestedBits >> 6) + 1;
        }
    }

    usize data[get_required_len()];
    
    constexpr void set(const usize index)
    {
        DebugAssert(index < RequestedBits, "invalid bit field");
        usize b = 1ULL << (index & 0x3F);
        data[(index / 64)] |= b;
    }

    constexpr void unset(const usize index)
    {
        DebugAssert(index < RequestedBits, "invalid bit field");
        data[(index / 64)] &= ~(1 << (index & 0x3F));
    }

    [[nodiscard]] constexpr bool is_set(const usize index) const
    { 
        DebugAssert(index < RequestedBits, "invalid bit field");
        return bool(
            data[(index / 64)] & (1ULL << (index & 0x3F))
        );
    }
    
    constexpr void clear() { ::new(data) usize[get_required_len()]{}; }
};
