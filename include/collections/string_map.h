#pragma once
#include "collections/string.h"
#include "collections/base_hash_map.h"
#include "collections/pair.h"
#include "mem/allocator.h"
#include "mem/utils.h"
#include "math/hash.h"


template<typename V>
struct StringHashMapEntry
{
    using KeyValue = Pair<StringView, V>;

    HashCode hash;
    KeyValue kv;

    StringHashMapEntry* prev;
    StringHashMapEntry* next;

    Mem::Allocator& allocator;

    template<typename... TArgs>
    StringHashMapEntry(Mem::Allocator& allocator, HashCode hash, Tuple<StringView> new_key, Tuple<TArgs...> args)
    : hash(hash), kv(Tuple(new_key), args), prev(), next(), allocator(allocator)
    {
        Slice<char> new_chars = allocator.array<char>(new_key.value.len);
        Mem::copy(new_chars, new_key.value);
        kv.first = new_chars;
    }

    ~StringHashMapEntry()
    {
        hash = Core::MaxValue<HashCode>;
        allocator.free(Slice(reinterpret_cast<u8*>(const_cast<char*>(kv.first.items)), 1));
    }

    template<typename Self>
    constexpr auto& keyvalue(this Self& self)
    {
        return self.kv;
    }

    constexpr HashCode hashvalue() const
    {
        return hash;
    }

    constexpr void set_hash(const HashCode& new_hash)
    {
        hash = new_hash;
    }
};


template<>
struct HashOfType<StringView>
{
    // FNV-1a
    [[nodiscard]] static constexpr HashCode hashfunc(const StringView& key)
    {
        return Math::Hash::fnv1a(
            Slice(
                reinterpret_cast<const u8*>(key.ptr()),
                key.len
            )
        );
    }
};

template<>
struct Comparator<StringView>
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
using StringMap = BaseHashMap<HashCode, StringHashMapEntry<V>, StringView, V>;

