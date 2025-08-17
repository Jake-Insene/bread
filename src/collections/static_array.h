#pragma once
#include "core/types.h"
#include "mem/utils.h"


template<typename T, usize N>
struct [[nodiscard]] StaticArray
{
    T items[N];
    usize count;

    static constexpr StaticArray with_count(usize item_count)
    {
        return StaticArray
        {
            .count = item_count,
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

    [[nodiscard]] constexpr T* begin() { return items; }
    [[nodiscard]] constexpr const T* begin() const { return items; }
    [[nodiscard]] constexpr T* end() { return items + count; }
    [[nodiscard]] constexpr const T* end() const { return items + count; }

    // funcs

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
