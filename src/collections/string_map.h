#pragma once
#include "collections/string.h"
#include "collections/pair.h"
#include "debug/assertion.h"
#include "mem/allocator.h"
#include "mem/utils.h"


/*
* A collection of items referenced as a string.
*/
template<typename T>
struct [[nodiscard]] StringMap
{
    using HashType = u64;
    using KeyValue = Pair<HashType, T>;

    static constexpr HashType InvalidHash = HashType(-1);
    static constexpr usize InvalidPos = usize(-1);
    static constexpr usize DefaultCapacity = 16;
    
    struct MapEntry
    {
        KeyValue kv;
        
        MapEntry* prev;
        MapEntry* next;
    };
    
    struct Iterator
    {
        MapEntry* entry;
        
        KeyValue& operator*() const { return entry->kv; }
        KeyValue* operator->() const { return &entry->kv; }
        
        Iterator& operator++()
        {
            if(entry)
            {
                entry = entry->next;
            }
            
            return *this;
        }
        
        Iterator& operator--()
        {
            if(entry)
            {
                entry = entry->prev;
            }
            
            return *this;
        }
        
        bool operator==(const Iterator& b) const { return entry == b.entry; }
		bool operator!=(const Iterator& b) const { return entry != b.entry; }

		explicit operator bool() const {
			return entry != nullptr;
		}
    };
    
    struct ConstIterator
    {
        const MapEntry* entry;
        
        KeyValue& operator*() const { return entry->kv; }
        KeyValue* operator->() const { return &entry->kv; }
        
        ConstIterator& operator++()
        {
            if(entry)
            {
                entry = entry->next;
            }
            
            return *this;
        }
        
        ConstIterator& operator--()
        {
            if(entry)
            {
                entry = entry->prev;
            }
            
            return *this;
        }
        
        bool operator==(const ConstIterator& b) const { return entry == b.entry; }
		bool operator!=(const ConstIterator& b) const { return entry != b.entry; }

		explicit operator bool() const { return entry != nullptr; }
    };
    
    mem::Allocator allocator;
    Slice<MapEntry*> entries;
    usize count;
    MapEntry* first;
    MapEntry* last;
    
    static StringMap<T> with_allocator(const mem::Allocator& allocator)
    {
        return StringMap
        {
            .allocator = allocator,
            .entries = {},
            .count = 0,
            .first = nullptr,
            .last = nullptr,
        };
    }
    
    static StringMap<T> with_size(const mem::Allocator& allocator, usize size)
    {
        return StringMap<T>
        {
            .allocator = allocator,
            .entries = allocator.array<MapEntry*>(size),
            .count = 0,
            .first = nullptr,
            .last = nullptr,
        };
    }
    
    void destroy()
    {
        if(entries.ptr())
        {
            for(auto entry : entries)
            {
                if(entry != nullptr)
                {
                    allocator.free(mem::to_bytes(Slice<MapEntry>(entry, 1)));
                }
            }
            allocator.free(mem::to_bytes(entries));
            entries = {};
        }
    }
    
    [[nodiscard]] constexpr Iterator begin() { return Iterator{.entry = first}; }
    [[nodiscard]] constexpr ConstIterator begin() const { return ConstIterator{.entry = first}; }
    [[nodiscard]] constexpr Iterator end() { return Iterator{.entry = nullptr}; }
    [[nodiscard]] constexpr ConstIterator end() const { return ConstIterator{.entry = nullptr}; }
    
    // non user funcs
    
    [[nodiscard]] bool _find_entry(const u64 hash, usize& pos) const
    {
        u64 i = hash & (entries.len - 1);
        usize dist = 0;
        
        while(true)
        {
            if(dist >= entries.len)
            {
                return false;
            }
            
            if(entries[i] != nullptr && entries[i]->kv.first == hash)
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
    
    [[nodiscard]] MapEntry* _insert_or_replace(StringView str, const T& value)
    {
        if (count >= entries.len)
        {
            resize(entries.len << 1);
        }
        else if (entries.len == 0)
        {
            resize(DefaultCapacity);
        }

        u64 hash = hashfunc(str);
        usize pos = InvalidPos;
        if(_find_entry(hash, pos))
        {
            entries[pos]->kv.second = value;
            return entries[pos];
        }
        else
        {
            usize i = hash & (entries.len - 1);
            while(true)
            {
                if(entries[i] == nullptr)
                {
                    MapEntry* entry = mem::from_bytes<MapEntry>(allocator.alloc(sizeof(MapEntry), alignof(MapEntry))).ptr();
                    entry->kv = KeyValue(hash, value);
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
                else if(entries[i]->kv.first == InvalidHash)
                {
                    MapEntry* entry = entries[i];
                    entry->kv = KeyValue(hash, value);
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
    }
    
    // user funcs
    
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
        
        Slice<MapEntry*> new_entries = allocator.array<MapEntry*>(new_size);

        for (MapEntry* e = first; e != nullptr; e = e->next)
        {
            u64 hash = e->kv.first;
            usize i = hash & (new_size - 1);
            while (new_entries[i] != nullptr)
            {
                i = (i + 1) % new_size;
            }
            new_entries[i] = e;
        }

        if (entries.ptr())
        {
            allocator.free(mem::to_bytes(entries));
        }

        entries = new_entries;
    }
    
    [[nodiscard]] bool has(StringView str) const
    {
        u64 hash = hashfunc(str);
        usize pos = InvalidPos;
        return _find_entry(hash, pos);
    }
    
    [[nodiscard]] T& get(StringView str)
    {
        u64 hash = hashfunc(str);
        usize pos = InvalidPos;
        (void)_find_entry(hash, pos);
        DebugAssert(pos != InvalidPos, "the item don't exists!");
        return entries[pos]->kv.second;
    }
    
    [[nodiscard]] const T& get(StringView str) const
    {
        u64 hash = hashfunc(str);
        usize pos = InvalidPos;
        (void)_find_entry(hash, pos);
        DebugAssert(pos != InvalidPos, "the item don't exists!");
        return entries[pos]->kv.second;
    }
    
    T& insert(StringView str, const T& value)
    {
        return _insert_or_replace(str, value)->kv.second;
    }
   
    void remove(StringView str)
    {
        u64 hash = hashfunc(str);
        usize pos = InvalidPos;
        (void)_find_entry(hash, pos);
        if (_find_entry(hash, pos) == false)
        {
            FailOn(true, "the item don't exists!");
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

        entry->kv.hash = InvalidHash;
        count--;
    }

    void clear()
    {
        for (auto& entry : entries)
        {
            if (entry)
            {
                entry->kv.first = InvalidHash;
            }
        }

        count = 0;
        first = last = nullptr;
    }
    
    // String map utilities

    // FNV-1a
    static u64 hashfunc(StringView key)
    {
        u64 hash = 0xcbf29ce484222325ULL;
        u64 i = 0;
        while(key.len != i)
        {
            hash ^= key[i];
            hash *= 0x100000001b3ULL;
            i++;
        }
        
        return hash;
    }
};
