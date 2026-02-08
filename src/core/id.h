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
    using Type = T;

    static constexpr Type InvalidID = MaxValue<T>;
    
    static constexpr ID invalid() { return ID(InvalidID); }
    
    Type id;
    
    constexpr ID() : id(InvalidID) {}
    constexpr explicit ID(Type id_value) : id(id_value) {}

    [[nodiscard]] constexpr bool operator==(const ID& other_id) const { return id == other_id.id; }
    
    [[nodiscard]] constexpr bool is_valid() const { return id != InvalidID; }

    [[nodiscard]] constexpr Type integer() const { return Type(id); }
};

