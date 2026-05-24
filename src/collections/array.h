#pragma once
#include "collections/base_iterator.h"
#include "mem/allocator.h"
#include "mem/utils.h"
#include "math/funcs.h"



/*
* Used to iterate over an array.
*/
template<typename T>
struct [[nodiscard]] ArrayIterator : BaseIterator<T>
{
    using Type = T;

    T* base;
    usize extent;
    
    T& operator*() const { return *base; }
    T* operator->() const { return base; }

    ArrayIterator& operator++()
    {
        base++;
        extent--;
        return *this;
    }

    ArrayIterator& operator--()
    {
        base--;
        extent++;
        return *this;
    }

    [[nodiscard]] bool operator==(const ArrayIterator& it) const
    {
        return base == it.base && extent == it.extent;
    }

    ArrayIterator begin() const { return *this; }
    ArrayIterator end() const { return ArrayIterator{ .base = base + extent, .extent = 0 }; }

    [[nodiscard]] usize distance(const ArrayIterator& it) const
    {
        return (extent - it.extent);
    }
};


/*
* A collection of linear memory that contains items of type T.
* You can add/remove/modify items.
*/
template<typename T>
struct [[nodiscard]] Array
{
    static constexpr usize DefaultCapacity = 4;

    using Type = T;
    using Iterator = ArrayIterator<Type>;

    Mem::Allocator* allocator;
    Slice<Type> items;
    usize count;

    static Array with_allocator(Mem::Allocator* allocator)
    {
        return
        {
            .allocator = allocator,
            .items = allocator->array<Type>(DefaultCapacity),
            .count = 0,
        };
    }

    static Array with_size(Mem::Allocator* allocator, usize size)
    {
        return
        {
            .allocator = allocator,
            .items = allocator->array<Type>(size),
            .count = 0,
        };
    }

    static Array from_items(Mem::Allocator* allocator, const Slice<Type>& items)
    {
        Array array =
        {
            .allocator = allocator,
            .items = allocator->array<Type>(items.len),
            .count = items.len,
        };

        Mem::copy(array.items, items);
        return array;
    }

    template<typename... TypeList>
    static constexpr Array from_list(Mem::Allocator* allocator, const TypeList... list)
    {
        static constexpr usize ListLen = sizeof...(list);
        Array array =
        {
            .allocator = allocator,
            .items = allocator->array<Type>(ListLen),
            .count = ListLen,
        };

        const Type list_array[] = { list... };
        Mem::copy(array.items, Slice(list_array, ListLen));

        return array;
    }

    void destroy()
    {
        if (items.ptr())
        {
            allocator->free(Mem::to_bytes(items));
        }
    }

    template<typename Self>
    Iterator iter(this Self& self)
    {
        return
        {
            .base = self.items.items,
            .extent = self.count,
        };
    }
    
    [[nodiscard]] bool is_empty() const { return count == 0; }
    
    void ensure_capacity(const usize required_capacity)
    {
        if(items.len >= required_capacity)
        {
            return;
        }
        
        usize new_cap = items.len + (items.len / 2);
        new_cap = Math::max(new_cap, required_capacity);
        
        if(!allocator->realloc(Mem::to_bytes(items), sizeof(Type) * new_cap, alignof(Type)))
        {
            Slice<Type> new_items = allocator->array<Type>(new_cap);
            if(items.ptr())
            {
                Mem::copy(new_items, items);
                allocator->free(Mem::to_bytes(items));
            }
                
            items = new_items;
        }
        else
        {
            items.len = new_cap;
            Slice<Type> items_to_construct = items.add(count);
            ConstructArray(items_to_construct.ptr(), items_to_construct.len);
        }
    }

    template<typename Self>
    [[nodiscard]] auto& get(this Self& self, usize index)
    {
        DebugAssert(index < self.count, "index out of range");
        return self.items[index];
    }

    template<typename Self>
    [[nodiscard]] auto& first(this Self& self)
    {
        DebugAssert(self.count > 0, "there is no 'first'");
        return self.items[0];
    }

    template<typename Self>
    [[nodiscard]] auto& last(this Self& self)
    {
        DebugAssert(self.count > 0, "there is no 'last'");
        return self.items[self.count - 1];
    }
    
    [[nodiscard]] Type& add(const Type& item)
    {
        ensure_capacity(count+1);
        items[count] = item;
        return items[count++];
    }

    void add_slice(const Slice<Type>& new_items)
    {
        usize _count = count;
        resize(count + new_items.len);
        Slice<Type> dest = items.add(_count);
        Mem::copy(dest, new_items);
    }

    void replace(Slice<Type> new_items)
    {
        resize(new_items.len);
        Mem::copy(items, new_items);
    }

    void remove_at(usize index)
    {
        DebugAssert(index < count && count != 0, "index out of range");
        DestructObject(items[index]);

        if (count == 1 || index == count - 1)
        {
            count--;
            return;
        }

        count--;
        Mem::copy(items.add(index), items.add(index + 1));
    }

    void remove(const Type& item)
    {
        auto it = iter().find(item);
        if (it == iter().end())
        {
            return;
        }

        remove_at(iter().distance(it));
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
    Slice<Type> slice(this Self& self) { return self.items.slice(self.count); }

    Array copy(Mem::Allocator* copy_allocator) const
    {
        return Array::from_items(copy_allocator, slice());
    }
};

