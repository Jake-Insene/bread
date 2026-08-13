#pragma once
#include "Collections/MapIterator.hpp"
#include "Collections/Pair.hpp"
#include "Mem/Allocator.hpp"
#include "Mem/Utils.hpp"



template<typename K, typename V, typename HashType>
struct BaseHashMapEntry
{
    using KeyValue = Pair<K, V>;

    HashType hash;
    KeyValue kv;

    BaseHashMapEntry* prev;
    BaseHashMapEntry* next;

    template<typename... TArgs1, typename... TArgs2>
    BaseHashMapEntry([[maybe_unused]] Mem::Allocator& allocator, HashType hash, Tuple<TArgs1...> args1, Tuple<TArgs2...> args2)
    : hash(hash), kv(args1, args2), prev(), next()
    {}

    template<typename Self>
    constexpr auto& keyvalue(this Self& self)
    {
        return self.kv;
    }

    constexpr const HashType& hashvalue() const
    {
        return hash;
    }

    constexpr void set_hash(const HashType& new_hash)
    {
        hash = new_hash;
    }
};

template<typename InHashType, typename InHashMapEntry, Core::Comparable K, typename V>
requires(Core::IsInteger<InHashType> && Core::IsUnsigned<InHashType>)
struct [[nodiscard]] BaseHashMap
{
    DisableCopy(BaseHashMap);
    DisableMove(BaseHashMap);
    
    using HashType = InHashType;
    using MapEntry = InHashMapEntry;
    using KeyValue = Pair<K, V>;
    using Iterator = MapIterator<InHashMapEntry, KeyValue>;
    using Hasher = Core::HashOfType<K>;
    using TComparator = Core::Comparator<K>;

    static constexpr HashType InvalidHash = Core::MaxValue<HashType>;
    static constexpr usize InvalidPos = Core::MaxValue<usize>;
    static constexpr usize DefaultCapacity = 16;

    Mem::Allocator& allocator;
    Slice<MapEntry*> entries;
    usize count;
    MapEntry* first;
    MapEntry* last;

    BaseHashMap(Mem::Allocator& allocator, usize initial_size)
    : allocator(allocator), entries(), count(), first(), last()
    {
        entries = Mem::from_bytes<MapEntry*>(
            allocator.alloc(sizeof(MapEntry*) * initial_size, alignof(MapEntry*))
        );
        Mem::zero(entries);
    }
    
    ~BaseHashMap()
    {
        if(entries.ptr() == nullptr)
        {
            return;
        }

        _destruct_objects();
        for(MapEntry* entry : entries)
        {
            if(entry != nullptr)
            {
                allocator.free(Mem::to_bytes(Slice(entry, 1)));
            }
        }
        allocator.free(Mem::to_bytes(entries));
        entries = {};
    }

    Iterator iter() const { return Iterator{ .entry = first }; }

    void resize(usize new_size)
    {
        if(entries.len == 0)
        {
            new_size = new_size > 0 ? new_size : DefaultCapacity;
            entries = allocator.array<MapEntry*>(new_size);
            return;
        }

        if(entries.len >= new_size)
        {
            return;
        }
        
        Slice new_entries = allocator.array<MapEntry*>(new_size);

        for (MapEntry* entry = first; entry != nullptr; entry = entry->next)
        {
            HashType hash = entry->hashvalue();
            usize index = hash & (new_size - 1);
            while (new_entries[index] != nullptr)
            {
                index = (index + 1) % new_size;
            }
            new_entries[index] = entry;
        }

        if (entries.ptr())
        {
            allocator.free(Mem::to_bytes(entries));
        }

        entries = new_entries;
    }

    [[nodiscard]] bool has(const K& key) const
    {
        HashType hash = Hasher::hashfunc(key);
        usize pos = InvalidPos;
        return _find_entry(hash, key, pos);
    }
    
    [[nodiscard]] V& get(const K& key)
    {
        HashType hash = Hasher::hashfunc(key);
        usize pos = InvalidPos;
        (void)_find_entry(hash, key, pos);
        DebugAssert(pos != InvalidPos, "the item don't exists!");
        return entries[pos]->keyvalue().second;
    }
    
    [[nodiscard]] const V& get(const K& key) const
    {
        HashType hash = Hasher::hashfunc(key);
        usize pos = InvalidPos;
        (void)_find_entry(hash, key, pos);
        DebugAssert(pos != InvalidPos, "the item don't exists!");
        return entries[pos]->keyvalue().second;
    }
    
    V& insert(const K& key, const V& value)
    {
        return _insert_or_replace(key, value)->keyvalue().second;
    }

    template<typename... TArgs>
    V& emplace(const K& key, TArgs&&... args)
    {
        return _try_emplace(key, Core::Forward<TArgs>(args)...)->keyvalue().second;
    }

    void remove(const K& key)
    {
        HashType hash = Hasher::hashfunc(key);
        usize pos = InvalidPos;
        if (!_find_entry(hash, key, pos))
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

        entry->set_hash(InvalidHash);
        --count;
    }

    void clear()
    {
        for (MapEntry* entry : entries)
        {
            if (entry)
            {
                entry->set_hash(InvalidHash);
            }
        }

        count = 0;
        first = last = nullptr;
    }

    [[nodiscard]] bool _find_entry(const HashType hash, const K& key, usize& pos) const
    {
        u64 index = hash & (entries.len - 1);
        usize dist = 0;

        while(true)
        {
            if(dist >= entries.len)
            {
                return false;
            }

            if (entries[index] != nullptr
                && entries[index]->hashvalue() == hash 
                && TComparator::compare(key, entries[index]->keyvalue().first)
            )
            {
                pos = index;
                return true;
            }

            ++dist;
            index = (index + 1) % entries.len;
        }
    }

    [[nodiscard]] MapEntry* _insert_or_replace(const K& key, const V& value)
    {
        if (count >= entries.len)
        {
            resize(entries.len << 1);
        }
        else if (entries.len == 0)
        {
            resize(DefaultCapacity);
        }

        HashType hash = Hasher::hashfunc(key);
        usize pos = InvalidPos;
        if (_find_entry(hash, key, pos))
        {
            entries[pos]->keyvalue().second = value;
            return entries[pos];
        }

        usize index = hash & (entries.len - 1);
        while(true)
        {
            if(entries[index] == nullptr)
            {
                MapEntry* entry = Mem::from_bytes<MapEntry>(
                    allocator.alloc(sizeof(MapEntry), alignof(MapEntry))
                ).ptr();
                Core::Mem::Placement(*entry, allocator, hash, Tuple(key), Tuple(value));

                entries[index] = entry;
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

                ++count;
                return entry;
            }
            
            if(entries[index]->hashvalue() == InvalidHash)
            {
                MapEntry* entry = entries[index];
                Core::Mem::Destruct(*entry);
                Core::Mem::Placement(*entry, allocator, hash, Tuple(key), Tuple(value));

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

                ++count;
                return entry;
            }
            
            index = (index + 1) % entries.len;
        }
    }

    template<typename... TArgs>
    [[nodiscard]] MapEntry* _try_emplace(const K& key, TArgs&&... args)
    {
        if (count >= entries.len)
        {
            resize(entries.len << 1);
        }
        else if (entries.len == 0)
        {
            resize(DefaultCapacity);
        }

        HashType hash = Hasher::hashfunc(key);
        usize pos = InvalidPos;
        if (_find_entry(hash, key, pos))
        {
            return entries[pos];
        }

        usize index = hash & (entries.len - 1);
        while(true)
        {
            if(entries[index] == nullptr)
            {
                MapEntry* entry = Mem::from_bytes<MapEntry>(
                    allocator.alloc(sizeof(MapEntry), alignof(MapEntry))
                ).ptr();
                Core::Mem::Placement(*entry, allocator, hash, Tuple(key),
                    Tuple<TArgs...>(Core::Forward<TArgs>(args)...));

                entries[index] = entry;
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

                ++count;
                return entry;
            }
            
            if(entries[index]->hashvalue() == InvalidHash)
            {
                MapEntry* entry = entries[index];
                Core::Mem::Destruct(*entry);
                Core::Mem::Placement(*entry, allocator, hash, Tuple(key),
                    Tuple<TArgs...>(Core::Forward<TArgs>(args)...));

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

                ++count;
                return entry;
            }
            
            index = (index + 1) % entries.len;
        }
    }

    void _destruct_objects()
    {
        for(Iterator it = iter().begin(); it != it.end(); ++it)
        {
            Core::Mem::Destruct(*it.entry);
        }
    }
};
