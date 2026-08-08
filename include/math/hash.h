#pragma once
#include "Core/Header.h"
#include "collections/slice.h"


namespace Math::Hash
{

// FNV-1a
static constexpr HashCode fnv1a(const Slice<const u8>& bytes)
{
    HashCode hash = 0xcbf29ce484222325ULL;
    for(usize index = 0; index < bytes.len; index++)
    {
        hash ^= bytes[index];
        hash *= 0x100000001b3ULL;
    }
    
    return hash;
}

}