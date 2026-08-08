#pragma once
#include "core/Templates.h"
#include "debug/assertion.h"


/*
* Set/unset a collection of bits.
* Useful for flags.
*/
template<usize RequestedBits = sizeof(usize) * 8>
struct [[nodiscard]] BitMask
{
    static constexpr usize _get_required_len()
    {
        if constexpr(RequestedBits <= 64)
        {
            return 1;
        }
        else
        {
            return (RequestedBits / 64) + 1;
        }
    }

    usize data[_get_required_len()];
    
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
    
    constexpr void clear() { ::new(data) usize[_get_required_len()]{}; }
};
