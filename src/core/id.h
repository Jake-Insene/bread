#pragma once
#include "core/types.h"
#include "core/values.h"

/*
* Use to create custom ID types.
*/
template<typename T, typename Tag>
struct [[nodiscard]] ID
{
    static constexpr T InvalidID = MaxValue<T>;
    
    T id;
    
    constexpr ID() : id(InvalidID) {}
    constexpr explicit ID(T id_value) : id(id_value) {}
    constexpr ID(const ID& other_id) : id(other_id.id) {}

    [[nodiscard]] constexpr operator T() const { return id; }
    
    [[nodiscard]] constexpr bool is_valid() const { return id != InvalidID; }
};

