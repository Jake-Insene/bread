#pragma once
#include "core/types.h"


// Layout
// 63-48 unused
// 47-40 chunk index
// 39-32 block index
// 31-0 block slot
struct ObjectID
{
    using InternalID = u64;
    
    static constexpr InternalID InvalidID = InternalID(-1);
    
    InternalID id;
    
    constexpr ObjectID()
        : id(InvalidID) {}
    constexpr ObjectID(InternalID slot, InternalID block, InternalID chunk)
        : id(slot | (block<<32) | (chunk<<40)) {}
    
    [[nodiscard]] bool constexpr operator==(ObjectID o) const { return id == o.id; }
    
    [[nodiscard]] constexpr operator InternalID() const { return id; }
    
    [[nodiscard]] constexpr u32 slot() const { return id & 0xFFFF'FFFF; }
    [[nodiscard]] constexpr u8 block() const { return (id >> 32) & 0xFF; }
    [[nodiscard]] constexpr u8 chunk() const { return (id >> 40) & 0xFF; }

    [[nodiscard]] constexpr bool is_valid() const
    {
        return id != InvalidID;
    }
};

static constexpr ObjectID InvalidObjectID = ObjectID(0xFFFF'FFFF, 0xFF, 0xFF);

