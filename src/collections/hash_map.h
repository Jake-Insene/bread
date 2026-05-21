#pragma once
#include "core/header.h"
#include "collections/map_iterator.h"
#include "collections/pair.h"
#include "mem/allocator.h"
#include "mem/utils.h"


// HashMapEntry
template<typename K, typename V, typename HashType>
struct HashMapEntry
{
    using KeyValue = Pair<K, V>;
    
    HashType hash;
    Pair<K, V> kv;

    HashMapEntry* prev;
    HashMapEntry* next;
};


/*
* A collection of items referenced as a key.
*/
template<Hashable K, typename V>
struct [[nodiscard]] HashMap
{
    using HashType = HashCode;
    using MapEntry = HashMapEntry<K, V, HashType>;
    using KeyValue = Pair<K, V>;
    using Iterator = MapIterator<MapEntry>;

    static constexpr HashType InvalidHash = MaxValue<HashType>;
    static constexpr usize InvalidPos = MaxValue<usize>;
    static constexpr usize DefaultCapacity = 16;

    Mem::Allocator* allocator;
    Slice<MapEntry*> entries;
    usize count;
    MapEntry* first;
    MapEntry* last;
    
    static HashMap with_allocator(Mem::Allocator* allocator)
    {
        return
        {
            .allocator = allocator,
            .entries = {},
            .count = 0,
            .first = nullptr,
            .last = nullptr,
        };
    }
    
    static HashMap with_size(Mem::Allocator* allocator, const usize size)
    {
        return
        {
            .allocator = allocator,
            .entries = allocator->array<MapEntry*>(size),
            .count = 0,
            .first = nullptr,
            .last = nullptr,
        };
    }
    
    void destroy()
    {
        if(entries.ptr() == nullptr)
        {
            return;
        }

        for(MapEntry* entry : entries)
        {
            if(entry != nullptr)
            {
                allocator->free(Mem::to_bytes(Slice<MapEntry>(entry, 1)));
            }
        }
        allocator->free(Mem::to_bytes(entries));
        entries = {};
    }

    Iterator iter() const { return Iterator{ .entry = first }; }

    void resize(usize new_size)
    {
        if(entries.len == 0)
        {
            new_size = new_size > 0 ? new_size : DefaultCapacity;
            entries = allocator->array<MapEntry*>(new_size);
            return;
        }

        if(entries.len >= new_size)
        {
            return;
        }
        
        Slice<MapEntry*> new_entries = allocator->array<MapEntry*>(new_size);

        for (MapEntry* e = first; e != nullptr; e = e->next)
        {
            HashType hash = e->hash;
            usize i = hash & (new_size - 1);
            while (new_entries[i] != nullptr)
            {
                i = (i + 1) % new_size;
            }
            new_entries[i] = e;
        }

        if (entries.ptr())
        {
            allocator->free(Mem::to_bytes(entries));
        }

        entries = new_entries;
    }
    
    [[nodiscard]] bool has(const K& k) const
    {
        HashType hash = HashOfType<K>::hashfunc(k);
        usize pos = InvalidPos;
        return _find_entry(hash, k, pos);
    }
    
    [[nodiscard]] V& get(const K& k)
    {
        HashType hash = HashOfType<K>::hashfunc(k);
        usize pos = InvalidPos;
        (void)_find_entry(hash, k, pos);
        DebugAssert(pos != InvalidPos, "the item don't exists!");
        return entries[pos]->kv.second;
    }
    
    [[nodiscard]] const V& get(const K& k) const
    {
        HashType hash = HashOfType<K>::hashfunc(k);
        usize pos = InvalidPos;
        (void)_find_entry(hash, k, pos);
        DebugAssert(pos != InvalidPos, "the item don't exists!");
        return entries[pos]->kv.second;
    }
    
    V& insert(const K& k, const V& value)
    {
        return _insert_or_replace(k, value)->kv.second;
    }

    void remove(const K& k)
    {
        HashType hash = HashOfType<K>::hashfunc(k);
        usize pos = InvalidPos;
        if (_find_entry(hash, k, pos) == false)
        {
            DebugAssert(false, "the item don't exists!");
        }

        MapEntry* entry = entries[pos];
        if (entry->prev)
        {
            entry->prev->next = entry->next;
        }

        if (entry->next)
        {
            entry->next->prev = entry->prev;
        }

        if (entry == first && entry == last)
        {
            first = nullptr;
            last = nullptr;
        }
        else if (entry == first)
        {
            first = entry->next;
        }
        else if (entry == last)
        {
            last = entry->prev;
        }

        entry->hash = InvalidHash;
        count--;
    }

    void clear()
    {
        for (MapEntry* entry : entries)
        {
            if (entry)
            {
                entry->hash = InvalidHash;
            }
        }

        count = 0;
        first = last = nullptr;
    }

    [[nodiscard]] bool _find_entry(const HashType hash, const K& k, usize& pos) const
    {
        u64 i = hash & (entries.len - 1);
        usize dist = 0;

        while(true)
        {
            if(dist >= entries.len)
            {
                return false;
            }

            if (entries[i] != nullptr
                && entries[i]->hash == hash 
                && HashOfType<K>::compare(k, entries[i]->kv.first)
            )
            {
                pos = i;
                return true;
            }

            dist++;
            i++;
            if (i == entries.len)
            {
                i = 0;
            }
        }
    }

    [[nodiscard]] MapEntry* _insert_or_replace(const K& k, const V& value)
    {
        if (count >= entries.len)
        {
            resize(entries.len << 1);
        }
        else if (entries.len == 0)
        {
            resize(DefaultCapacity);
        }

        HashType hash = HashOfType<K>::hashfunc(k);
        usize pos = InvalidPos;
        if (_find_entry(hash, k, pos))
        {
            entries[pos]->kv.second = value;
            return entries[pos];
        }

        usize i = hash & (entries.len - 1);
        while(true)
        {
            if(entries[i] == nullptr)
            {
                MapEntry* entry = Mem::from_bytes<MapEntry>(
                    allocator->alloc(sizeof(MapEntry), alignof(MapEntry))
                ).ptr();
                entry->hash = hash;
                entry->kv = KeyValue(k, value);
                entry->prev = nullptr;
                entry->next = nullptr;

                entries[i] = entry;
                if(first == nullptr)
                {
                    first = entry;
                    last = entry;
                }
                else
                {
                    last->next = entry;
                    entry->prev = last;
                    last = entry;
                }

                count++;
                return entry;
            }
            
            if(entries[i]->hash == InvalidHash)
            {
                MapEntry* entry = entries[i];
                entry->hash = hash;
                entry->kv = KeyValue(k, value);
                entry->prev = nullptr;
                entry->next = nullptr;

                if (first == nullptr)
                {
                    first = entry;
                    last = entry;
                }
                else
                {
                    last->next = entry;
                    entry->prev = last;
                    last = entry;
                }

                count++;
                return entry;
            }
            
            i++;
            if (i == entries.len)
            {
                i = 0;
            }
        }
    }
};
