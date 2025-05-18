#pragma once
#include "core/types.h"
#include "core/macros.h"
#include "core/templates.h"

template<typename T>
struct [[nodiscard]] Slice
{
    T* items = nullptr;
    usize len = 0;
    
    constexpr Slice() : items(nullptr), len(0) {}
    
    constexpr Slice(const Slice<RemoveConst<T>>& slice) : items(slice.items), len(slice.len) {}

    template<usize Len>
    constexpr Slice(T(&_items)[Len])
    {
        items = _items;
        len = Len;
    }

    constexpr Slice(T* _items, const usize _len)
    {
        items = _items;
        len = _len;
    }

    constexpr Slice& operator=(const Slice<RemoveConst<T>>& slice)
    {
        items = slice.items;
        len = slice.len;
        return *this;
        
    }
    
    [[nodiscard]] constexpr T& operator[](const usize index)
    {
        DebugAssert(index < len, "index out of range");
        return items[index];
    }
    
    [[nodiscard]] constexpr const T& operator[](const usize index) const
    {
        DebugAssert(index < len, "index out of range");
        return items[index];
    }

    [[nodiscard]] constexpr T* begin() { return items; }
    [[nodiscard]] constexpr T* begin() const { return items; }

    [[nodiscard]] constexpr T* end() { return items + len; }
    [[nodiscard]] constexpr const T* end() const { return items + len; }

    [[nodiscard]] constexpr T* ptr() { return items; };
    [[nodiscard]] constexpr const T* ptr() const { return items; };

    [[nodiscard]] constexpr bool null() const { return items == nullptr; }

    constexpr Slice<T> add(const usize offset) const
    {
        DebugAssert(ptr() && ((len - offset) > 0 || (len - offset) <= len), "invalid offset");
        return Slice(items + offset, len - offset);
    }

    constexpr Slice<T> sub(const usize offset) const
    {
        DebugAssert(ptr() && ((len + offset) >= len), "invalid offset");
        return Slice(items - offset, len + offset);
    }
};
