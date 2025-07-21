#pragma once
#include "core/types.h"

// Layout
// 0-31 block slot
// 32-39 block index
// 40-47 chunk index
// 48-63 unused


struct ObjectID
{
    using ID = u64;
    
    static constexpr ID InvalidID = ID(-1);
    
    ID id;
    
    constexpr ObjectID()
        : id(InvalidID) {}
    constexpr ObjectID(ID slot, ID block, ID chunk)
        : id(slot | (block<<32) | (chunk<<40)) {}
    
    [[nodiscard]] bool constexpr operator==(ObjectID o) const { return id == o.id; }
    
    [[nodiscard]] constexpr operator ID() const { return id; }
    
    [[nodiscard]] constexpr u32 slot() const { return id & 0xFFFF'FFFF; }
    [[nodiscard]] constexpr u8 block() const { return (id >> 32) & 0xFF; }
    [[nodiscard]] constexpr u8 chunk() const { return (id >> 40) & 0xFF; }

    [[nodiscard]] constexpr bool is_valid() const
    {
        return id != InvalidID;
    }
};

static constexpr ObjectID InvalidObjectID = ObjectID(0xFFFF'FFFF, 0xFF, 0xFF);

