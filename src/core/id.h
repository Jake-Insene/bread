#pragma once
#include "core/types.h"
#include "core/values.h"


/*
* Use to create custom ID types.
* 
* @param Tag Specify a tag to make unique ID types
*/
template<typename T, typename Tag>
struct [[nodiscard]] ID
{
    static constexpr T InvalidID = MaxValue<T>;
    
    static constexpr ID invalid() { return ID(InvalidID); }
    
    T id;
    
    constexpr ID() : id(InvalidID) {}
    constexpr explicit ID(T id_value) : id(id_value) {}

    [[nodiscard]] constexpr operator T() const { return id; }

    [[nodiscard]] constexpr bool operator==(const ID& other_id) const { return id == other_id; }
    
    [[nodiscard]] constexpr bool is_valid() const { return id != InvalidID; }
};

