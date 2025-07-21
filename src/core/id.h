#pragma once
#include "core/types.h"

template<typename T = u64>
struct [[nodiscard]] ID
{
    static constexpr T InvalidID = T(-1);
    
    T id;
    
    constexpr ID() : id(InvalidID) {}
    constexpr ID(T _id) : id(_id) {}
    
    [[nodiscard]] constexpr operator T() const { return id; }
};

