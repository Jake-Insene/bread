#pragma once
#include "Core/Types.hpp"
#include "Core/Values.hpp"


namespace Collections
{

/**
* Use to create custom ID types.
* 
* @tparam T The underlying ID type.
* @tparam Tag Specify a tag to make unique ID types
*/
template<typename T, typename Tag>
struct [[nodiscard]] ID
{
    using Type = T;

    static constexpr Type InvalidID = Core::MaxValue<T>;
    
    static constexpr ID invalid() { return ID(InvalidID); }
    
    Type id;
    
    constexpr ID() : id(InvalidID) {}
    constexpr explicit ID(Type id_value) : id(id_value) {}

    [[nodiscard]] constexpr bool operator==(const ID& other_id) const { return id == other_id.id; }
    
    [[nodiscard]] constexpr bool is_valid() const { return id != InvalidID; }

    [[nodiscard]] constexpr Type integer() const { return Type(id); }
};

}
