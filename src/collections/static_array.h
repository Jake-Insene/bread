#pragma once
#include "core/types.h"
#include "collections/base_iterator.h"
#include "mem/utils.h"



/*
* Used to iterate over an static array.
*/
template<typename T>
struct [[nodiscard]] StaticArrayIterator : BaseIterator<StaticArrayIterator<T>, T>
{
    using Type = T;

    T* base;
    usize extent;

    constexpr StaticArrayIterator(T* base, usize extent) : base(base), extent(extent) {}

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
    constexpr StaticArrayIterator end() const { return StaticArrayIterator(base + extent, 0); }

    [[nodiscard]] usize distance(const StaticArrayIterator& it) const
    {
        return (extent - it.extent);
    }
};

template<typename T, usize N>
struct [[nodiscard]] StaticArray
{
    using Type = T;
    using Iterator = StaticArrayIterator<Type>;

    T items[N];
    usize count;

    static constexpr StaticArray with_count(usize item_count)
    {
        return StaticArray
        {
            .items = {},
            .count = item_count
        };
    }

    static constexpr StaticArray from_items(Slice<T> items)
    {
		DebugAssert(N >= items.len, "Static Array size is too small for the provided items");
        StaticArray array =
        {
            .count = items.len,
        };

        Slice<T> dest = Slice(array.items, N);
        mem::copy(dest, items);
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

        const T list_array[] = { list... };
        Slice<T> dest = Slice(array.items, N);
        mem::copy(dest, Slice(list_array, ListLen));

        return array;
    }

    template<typename Self>
    constexpr Iterator iter(this Self& self)
    {
        return Iterator(self.items, self.count);
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

    constexpr void add_slice(Slice<T> new_items)
    {
		DebugAssert(count + new_items.len <= N, "StaticArray is full, cannot add more items");
        usize _count = count;
        Slice<T> dest = items.add(_count);
        mem::copy(dest, new_items);
        count += new_items.len;
    }

    constexpr void replace(Slice<T> new_items)
    {
		DebugAssert(new_items.len <= N, "StaticArray is too small for the provided items");
        Slice<T> dest = Slice(items, N);
        mem::copy(dest, new_items);
    }

    constexpr void remove(usize index)
    {
        DebugAssert(index < count && count != 0, "index out of range");
        if (count == 1)
        {
            count--;
        }
        else
        {
            count--;
            Slice<T> dest = Slice(items, N);
            Slice<T> src = Slice(items, N);
            mem::copy(dest.add(index), src.add(index + 1));
        }
    }

    constexpr void remove_equal(const T& item)
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

    constexpr void clear()
    {
        count = 0;
    }

    constexpr Slice<T> slice() { return Slice(items.items, count); }
};
