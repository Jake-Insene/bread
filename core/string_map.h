#pragma once
#include "core/string.h"
#include "mem/allocator.h"
#include "mem/utils.h"

#include <memory>

template<typename T>
struct [[nodiscard]] StringMap
{
    static constexpr u64 EmptyHash = 0;
    static constexpr usize InvalidPos = usize(-1);
    static constexpr usize DefaultCapacity = 4;
    
    struct MapKeyValue
    {
        T value;
        u64 hash;
    };
    
    struct MapEntry
    {
        MapKeyValue kv;
        
        MapEntry* prev;
        MapEntry* next;
    };
    
    struct Iterator
    {
        MapEntry* entry;
        
        MapKeyValue& operator*() const { return entry->kv; }
        MapKeyValue* operator->() const { return &entry->kv; }
        
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
        
        MapKeyValue& operator*() const { return entry->kv; }
        MapKeyValue* operator->() const { return &entry->kv; }
        
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
    
    mem::Allocator allocator{};
    Slice<MapEntry*> entries{};
    usize count = 0;
    MapEntry* first = nullptr;
    MapEntry* last = nullptr;
    
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
            .entries = mem::from_bytes<MapEntry*>(allocator.alloc(sizeof(MapEntry*) * size, alignof(MapEntry))),
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
                if(entry)
                {
                    allocator.free(mem::to_bytes(Slice<MapEntry>(entry, 1)));
                }
            }
            allocator.free(mem::to_bytes(entries));
        }
    }
    
    [[nodiscard]] constexpr Iterator begin() { return Iterator{.entry = first}; }
    [[nodiscard]] constexpr ConstIterator begin() const { return ConstIterator{.entry = first}; }
    [[nodiscard]] constexpr Iterator end() { return Iterator{.entry = nullptr}; }
    [[nodiscard]] constexpr ConstIterator end() const { return ConstIterator{.entry = nullptr}; }
    
    // non user funcs
    
    [[nodiscard]] bool _find_entry(const u64 hash, usize& pos)
    {
        u64 i = hash & (entries.len - 1);
        usize dist = 0;
        
        while(true)
        {
            if(dist >= entries.len)
            {
                return false;
            }
            
            if(entries[i] != nullptr && entries[i]->kv.hash == hash)
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
        u64 hash = hashfunc(str);
        
        usize pos = InvalidPos;
        if(_find_entry(hash, pos))
        {
            entries[pos]->kv.value = value;
            return entries[pos];
        }
        else
        {
            if(count >= entries.len)
            {
                resize(entries.len << 1);
            }
            else if(entries.len == 0)
            {
                resize(DefaultCapacity);
            }
            
            usize i = hash & (entries.len - 1);
            while(true)
            {
                if(entries[i] == nullptr)
                {
                    MapEntry* entry = mem::from_bytes<MapEntry>(allocator.alloc(sizeof(MapEntry), alignof(MapEntry))).ptr();
                    entry->kv.hash = hash;
                    entry->kv.value = value;
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
                else if(entries[i]->kv.hash == EmptyHash)
                {
                    entries[i]->kv.hash = hash;
                    entries[i]->kv.value = value;
                    count++;
                    return entries[i];
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
            entries = mem::from_bytes<MapEntry*>(allocator.alloc(sizeof(MapEntry*) * new_size, alignof(MapEntry*)));
            return;
        }

        if(count >= new_size)
        {
            return;
        }
        
        if(!allocator.realloc(mem::to_bytes(entries), sizeof(MapEntry*) * new_size, alignof(MapEntry*)))
        {
            auto new_items = mem::from_bytes<MapEntry*>(allocator.alloc(sizeof(MapEntry*) * new_size, alignof(MapEntry*)));
            if(entries.ptr())
            {
                std::memcpy(new_items.ptr(), entries.ptr(), sizeof(MapEntry) * entries.len);
                allocator.free(mem::to_bytes(entries));
            }
            
            entries = new_items;
        }
        else
        {
            entries.len = new_size;
        }
    }
    
    [[nodiscard]] bool has(StringView str)
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
        DebugAssert(pos != InvalidPos, "The item don't exists!");
        return entries[pos]->kv.value;
    }
    
    [[nodiscard]] const T& get(StringView str) const
    {
        u64 hash = hashfunc(str);
        usize pos = InvalidPos;
        (void)_find_entry(hash, pos);
        DebugAssert(pos != InvalidPos, "The item don't exists!");
        return entries[pos]->kv.value;
    }
    
    T& insert(StringView str, const T& value)
    {
        return _insert_or_replace(str, value)->kv.value;
    }
    
    // string map utilities
    
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
