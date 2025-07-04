#pragma once
#include "core/types.h"
#include "mem/allocator.h"
#include "mem/utils.h"


template<typename T>
struct [[nodiscard]] Array
{
    static constexpr usize DefaultCapacity = 4;
    
    mem::Allocator allocator{};
    Slice<T> items{};
    usize count{};
    
    static Array with_allocator(const mem::Allocator& allocator)
    {
        auto items = allocator.array<T>(DefaultCapacity);
        return Array
        {
            .allocator = allocator,
            .items = items,
            .count = 0,
        };
    }
    
    static Array with_size(const mem::Allocator& allocator, const usize size)
    {
        return Array
        {
            .allocator = allocator,
            .items = allocator.array<T>(size),
            .count = 0,
        };
    }

    static Array from_items(const mem::Allocator& allocator, Slice<T> items)
    {
        Array array =
        {
            .allocator = allocator,
            .items = allocator.array<T>(items.len),
            .count = items.len,
        };

        mem::copy(array.items, items);
        return array;
    }
    
    void destroy()
    {
        if(items.ptr())
        {
            allocator.free(mem::to_bytes(items));
        }
    }
    
    [[nodiscard]] constexpr T& operator[](const usize index)
    {
        DebugAssert(index < count, "index out of range");
        return items[index];
    }
    
    [[nodiscard]] constexpr const T& operator[](const usize index) const
    {
        DebugAssert(index < count, "index out of range");
        return items[index];
    }
    
    [[nodiscard]] constexpr T* begin() { return items.items; }
    [[nodiscard]] constexpr const T* begin() const { return items.items; }
    [[nodiscard]] constexpr T* end() { return items.items + count; }
    [[nodiscard]]constexpr const T* end() const { return items.items + count; }
    
    // funcs
    
    void ensure_capacity(const usize required_capacity)
    {
        if(items.len >= required_capacity)
        {
            return;
        }
        
        usize new_cap = items.len + items.len / 2;
        if(new_cap < required_capacity)
        {
            new_cap = required_capacity;
        }
            
        if(!allocator.realloc(mem::to_bytes(items), sizeof(T) * new_cap, alignof(T)))
        {
            auto new_items = allocator.array<T>(new_cap);
            if(items.ptr())
            {
                mem::copy(new_items, items);
                allocator.free(mem::to_bytes(items));
            }
                
            items = new_items;
        }
        else
        {
            items.len = new_cap;
            allocator.construct_array(items.add(count));
        }
    }
    
    [[nodiscard]] T& add(const T& item)
    {
        ensure_capacity(count+1);
        items[count] = item;
        return items[count++];
    }

    void add_slice(Slice<T> new_items)
    {
        usize _count = count;
        resize(count + new_items.len);
        Slice<T> dest = items.add(_count);
        mem::copy(dest, new_items);
    }

    void replace(Slice<T> new_items)
    {
        resize(new_items.len);
        mem::copy(items, new_items);
    }

    void remove(usize index)
    {
        DebugAssert(index < count && count != 0, "index out of range");
        if (count == 1)
        {
            count--;
        }
        else
        {
            count--;
            mem::copy(items.add(index), items.add(index + 1));
        }
    }

    void remove_equal(const T& item)
    {
        for (usize i = 0; i < count; i++)
        {
            if (items[i] == item)
            {
                remove(i);
                return;
            }
        }
    }
    
    void resize(const usize new_size)
    {
        ensure_capacity(new_size);
        count = new_size;
    }
    
    void clear()
    {
        count = 0;
    }
    
    Slice<T> slice() { return Slice(items.items, count); }
    Slice<T> slice() const { return Slice(items.items, count); }
    Slice<T> get_slice(usize start, usize num) const { return Slice(items.items + start, num); }
};
