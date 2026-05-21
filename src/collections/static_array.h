#pragma once
#include "collections/base_iterator.h"
#include "mem/utils.h"



/*
* Used to iterate over an static array.
*/
template<typename T>
struct [[nodiscard]] StaticArrayIterator : BaseIterator<T>
{
    using Type = T;

    T* base;
    usize extent;

    constexpr T& operator*() const { return *base; }
    constexpr T* operator->() const { return base; }

    constexpr StaticArrayIterator& operator++()
    {
        base++;
        extent--;
        return *this;
    }

    constexpr StaticArrayIterator& operator--()
    {
        base--;
        extent++;
        return *this;
    }

    [[nodiscard]] constexpr bool operator==(const StaticArrayIterator& it) const
    {
        return base == it.base && extent == it.extent;
    }

    constexpr StaticArrayIterator begin() const { return *this; }
    constexpr StaticArrayIterator end() const { return StaticArrayIterator{ .base = base + extent, .extent = 0 }; }

    [[nodiscard]] usize distance(const StaticArrayIterator& it) const
    {
        return (extent - it.extent);
    }
};


/*
* A collection of limited linear memory that contains items of type T.
* You can add/remove/modify items.
*/
template<typename T, usize N>
struct [[nodiscard]] StaticArray
{
    using Type = T;
    using Iterator = StaticArrayIterator<Type>;

    T items[N];
    usize count;

    static constexpr StaticArray with_count(usize item_count)
    {
        return
        {
            .items = {},
            .count = item_count
        };
    }

    static constexpr StaticArray from_items(Slice<Type> items)
    {
		DebugAssert(N >= items.len, "Static Array size is too small for the provided items");
        StaticArray array =
        {
            .count = items.len,
        };

        Slice<Type> dest = Slice(array.items, N);
        Mem::copy(dest, items);
        return array;
    }

    template<typename... TList>
    static constexpr StaticArray from_list(TList&&... list)
    {
        static constexpr usize ListLen = sizeof...(list);
        DebugAssert(N >= ListLen, "Static Array size is too small for the provided items");
        StaticArray array =
        {
            .items = {},
            .count = ListLen,
        };

        const Type list_array[] = { list... };
        Slice<Type> dest = Slice(array.items, N);
        Mem::copy(dest, Slice(list_array, ListLen));

        return array;
    }

    template<typename Self>
    constexpr Iterator iter(this Self& self)
    {
        return
        {
            .base = self.items,
            .extent = self.count,
        };
    }

    [[nodiscard]] constexpr bool is_empty() const { return count == 0; }

    template<typename Self>
    [[nodiscard]] constexpr auto& get(this Self& self, usize index)
    {
        DebugAssert(index < self.count, "index out of range");
        return self.items[index];
    }

    [[nodiscard]] constexpr T& add(const T& item)
    {
        DebugAssert(count < N, "StaticArray is full, cannot add more items");
        items[count] = item;
        return items[count++];
    }

    constexpr void add_slice(Slice<Type> new_items)
    {
		DebugAssert(count + new_items.len <= N, "StaticArray is full, cannot add more items");
        usize _count = count;
        Slice<Type> dest = items.add(_count);
        Mem::copy(dest, new_items);
        count += new_items.len;
    }

    constexpr void replace(Slice<Type> new_items)
    {
		DebugAssert(new_items.len <= N, "StaticArray is too small for the provided items");
        Slice<Type> dest = Slice(items, N);
        Mem::copy(dest, new_items);
    }

    constexpr void remove_at(usize index)
    {
        DebugAssert(index < count && count != 0, "index out of range");
        DestructObject(items[index]);

        if (count == 1 || index == count - 1)
        {
            count--;
            return;
        }

        count--;
        Slice<Type> dest = Slice(items, N);
        Slice<Type> src = Slice(items, N);
        Mem::copy(dest.add(index), src.add(index + 1));
    }

    constexpr void remove(const T& item)
    {
        auto it = iter().find(item);
        if (it == iter().end())
        {
            return;
        }

        remove_at(iter().distance(it));
    }

    constexpr void clear()
    {
        count = 0;
    }

    constexpr Slice<Type> slice() { return Slice(items.items, count); }

    StaticArray copy() const
    {
        return StaticArray::from_items(slice());
    }
};
