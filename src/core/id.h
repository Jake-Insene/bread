#pragma once
#include "core/types.h"

/*
* Use to create custom ID types.
*/
template<typename T>
struct [[nodiscard]] ID
{
    static constexpr T InvalidID = T(-1);
    
    T id;
    
    constexpr ID() : id(InvalidID) {}
    constexpr ID(T id_value) : id(id_value) {}

    [[nodiscard]] constexpr operator T() const { return id; }
    
    [[nodiscard]] constexpr bool is_valid() const { return id != InvalidID; }
};

