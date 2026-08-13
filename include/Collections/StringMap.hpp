#pragma once
#include "Collections/String.hpp"
#include "Collections/BaseHashMap.hpp"
#include "Collections/Pair.hpp"
#include "Mem/Allocator.hpp"
#include "Mem/Utils.hpp"
#include "math/hash.h"


template<typename V>
struct StringHashMapEntry
{
    using KeyValue = Pair<StringView, V>;

    Core::HashCode hash;
    KeyValue kv;

    StringHashMapEntry* prev;
    StringHashMapEntry* next;

    Mem::Allocator& allocator;

    template<typename... TArgs>
    StringHashMapEntry(Mem::Allocator& allocator, Core::HashCode hash, Tuple<const StringView&> new_key, Tuple<TArgs...> args)
    : hash(hash), kv(Tuple(new_key), args), prev(), next(), allocator(allocator)
    {
        Slice<char> new_chars = allocator.array<char>(new_key.value.len);
        Mem::copy(new_chars, new_key.value);
        kv.first = StringView(new_chars.items, new_chars.len);
    }

    ~StringHashMapEntry()
    {
        hash = Core::MaxValue<Core::HashCode>;
        allocator.free(Slice(reinterpret_cast<u8*>(const_cast<char*>(kv.first.items)), 1));
    }

    template<typename Self>
    constexpr auto& keyvalue(this Self& self)
    {
        return self.kv;
    }

    constexpr Core::HashCode hashvalue() const
    {
        return hash;
    }

    constexpr void set_hash(const Core::HashCode& new_hash)
    {
        hash = new_hash;
    }
};


template<>
struct Core::HashOfType<StringView>
{
    // FNV-1a
    [[nodiscard]] static constexpr Core::HashCode hashfunc(const StringView& key)
    {
        return Math::Hash::fnv1a(
            ::Mem::to_const_bytes(key)
        );
    }
};

template<>
struct Core::Comparator<StringView>
{
    [[nodiscard]] static constexpr bool compare(const StringView& value1, const StringView& value2)
    {
        return value1.equals(value2);
    }
};

/*
* A collection of items referenced as a string.
*/
template<typename V>
using StringMap = BaseHashMap<Core::HashCode, StringHashMapEntry<V>, StringView, V>;

