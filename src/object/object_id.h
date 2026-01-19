#pragma once
#include "core/types.h"


/*
* ObjectID to use with object allocators like pools or custom ones.
* Always 64 bits.
*/
struct ObjectID
{
    using InternalID = u64;
    
    static constexpr InternalID InvalidID = InternalID(-1);
    
    InternalID id;
    
    constexpr ObjectID()
        : id(InvalidID) {}
    constexpr ObjectID(InternalID id)
        : id(id) {}
    
    [[nodiscard]] bool constexpr operator==(ObjectID o) const { return id == o.id; }
    
    [[nodiscard]] constexpr operator InternalID() const { return id; }

    [[nodiscard]] constexpr bool is_valid() const
    {
        return id != InvalidID;
    }
};


