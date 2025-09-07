#pragma once
#include "core/types.h"
#include "core/templates.h"

template<typename T>
struct [[nodiscard]] Slice
{
    T* items;
    usize len;
    
    constexpr Slice() : items(nullptr), len(0) {}
    
    constexpr Slice(const Slice<RemoveConst<T>>& slice) : items(slice.items), len(slice.len) {}

    template<usize N>
    constexpr Slice(T(&_items)[N])
        : items(_items), len(N)
    {}

    constexpr Slice(T* _items, const usize _len)
        : items(_items), len(_len)
    {}

    constexpr Slice& operator=(const Slice<RemoveConst<T>>& slice)
    {
        items = slice.items;
        len = slice.len;
        return *this;
    }
    
    [[nodiscard]] constexpr T& operator[](const usize index);
    [[nodiscard]] constexpr const T& operator[](const usize index) const;

    [[nodiscard]] constexpr T* begin() { return items; }
    [[nodiscard]] constexpr T* begin() const { return items; }

    [[nodiscard]] constexpr T* end() { return items + len; }
    [[nodiscard]] constexpr const T* end() const { return items + len; }

    [[nodiscard]] constexpr T* ptr() { return items; };
    [[nodiscard]] constexpr const T* ptr() const { return items; };

    [[nodiscard]] constexpr bool null() const { return items == nullptr; }

    constexpr Slice<T> add(const usize offset) const;
    constexpr Slice<T> sub(const usize offset) const;

    constexpr Slice<T> slice(usize count) const;
};

#include "debug/assertion.h"

template<typename T>
constexpr Slice<T> Slice<T>::slice(usize count) const
{
    DebugAssert(count <= len, "index out of range");
    return Slice<T>(items, count);
}

template<typename T>
[[nodiscard]] constexpr T& Slice<T>::operator[](const usize index)
{
    DebugAssert(index < len, "index out of range");
    return items[index];
}

template<typename T>
[[nodiscard]] constexpr const T& Slice<T>::operator[](const usize index) const
{
    DebugAssert(index < len, "index out of range");
    return items[index];
}

template<typename T>
constexpr Slice<T> Slice<T>::add(const usize offset) const
{
    DebugAssert(ptr() && ((len - offset) > 0 || (len - offset) <= len), "invalid offset");
    return Slice(items + offset, len - offset);
}

template<typename T>
constexpr Slice<T> Slice<T>::sub(const usize offset) const
{
    DebugAssert(ptr() && ((len + offset) >= len), "invalid offset");
    return Slice(items - offset, len + offset);
}
