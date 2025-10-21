#pragma once
#include "collections/base_iterator.h"
#include "mem/allocator.h"
#include "mem/utils.h"


/*
* Used to iterate over an array.
*/
template<typename T>
struct [[nodiscard]] ArrayIterator : BaseIterator<ArrayIterator<T>, T>
{
    using Type = T;

    T* base;
    usize extend;
    
    ArrayIterator(T* base, usize extend) : base(base), extend(extend) {}

    T& operator*() const { return *base; }
    T* operator->() const { return base; }

    ArrayIterator& operator++()
    {
        base++;
        extend--;
        return *this;
    }

    ArrayIterator& operator--()
    {
        base--;
        extend++;
        return *this;
    }

    [[nodiscard]] bool operator==(const ArrayIterator& it) const
    {
        return base == it.base && extend == it.extend;
    }

    ArrayIterator begin() const { return *this; }
    ArrayIterator end() const { return ArrayIterator(base + extend, 0); }

    [[nodiscard]] usize distance(const ArrayIterator& it) const
    {
        return (extend - it.extend);
    }
};


/*
* A collection of linear memory that contains items of type T.
* You can add/remove/modify items and clear it.
*/
template<typename T>
struct [[nodiscard]] Array
{
    static constexpr usize DefaultCapacity = 4;

    mem::Allocator allocator = {};
    Slice<T> items = {};
    usize count = 0;

    static Array with_allocator(const mem::Allocator& allocator)
    {
        return Array
        {
            .allocator = allocator,
            .items = allocator.array<T>(DefaultCapacity),
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

    template<typename... TList>
    static constexpr Array from_list(const mem::Allocator& allocator, const TList... list)
    {
        static constexpr usize ListLen = sizeof...(list);
        Array array =
        {
            .allocator = allocator,
            .items = allocator.array<T>(ListLen),
            .count = ListLen,
        };

        const T list_array[] = { list... };
        mem::copy(array.items, Slice(list_array, ListLen));

        return array;
    }

    void destroy()
    {
        if (items.ptr())
        {
            allocator.free(mem::to_bytes(items));
        }
    }

    ArrayIterator<T> iter() const
    {
        return ArrayIterator<T>(items.items, count);
    }
    
    [[nodiscard]] bool is_empty() const { return count == 0; }
    
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

    template<typename Self>
    [[nodiscard]] auto& get(this Self& self, usize index)
    {
        DebugAssert(index < self.count, "index out of range");
        return self.items[index];
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

    void remove_it(const ArrayIterator<T>& it)
    {
        if (it == iter().end())
        {
            return;
        }

        remove(iter().distance(it));
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
    
    template<typename Self>
    Slice<T> slice(this Self& self) { return self.items.slice(self.count); }
};
