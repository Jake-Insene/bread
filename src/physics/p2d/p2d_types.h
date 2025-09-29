#pragma once
#include "collections/hash_map.h"
#include "math/vec2.h"
#include "physics/physics_2d.h"



struct CollisionID
{
    Physics2D::BodyID id1;
    Physics2D::BodyID id2;
};

using PhysicsTileCoord = Vector2I;

template<>
struct HashOfType<CollisionID>
{
    [[nodiscard]] static constexpr u64 hashfunc(const CollisionID& k)
    {
        return k.id1.id | (u64(k.id2.id) << 32);
    }

    [[nodiscard]] static constexpr bool compare(const CollisionID& k1, const CollisionID& k2)
    {
        return (k1.id1 == k2.id1 && k1.id2 == k2.id2) || (k1.id1 == k2.id2 && k1.id2 == k2.id1);
    }
};

template<>
struct HashOfType<PhysicsTileCoord>
{
    [[nodiscard]] static constexpr u64 hashfunc(const PhysicsTileCoord& k)
    {
        return k.x | (u64(k.y) << 32);
    }

    [[nodiscard]] static constexpr bool compare(const PhysicsTileCoord& k1, const PhysicsTileCoord& k2)
    {
        return k1.x == k2.x && k1.y == k2.y;
    }
};