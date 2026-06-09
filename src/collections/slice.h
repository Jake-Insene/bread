#pragma once
#include "core/types.h"
#include "core/templates.h"
#include "debug/assertion.h"



/*
* Contains a linear collection of T elements.
* T The type of the elements in the slice.
* Read and Write are allowed.
*/
template<typename T>
struct [[nodiscard]] Slice
{
    using Type = T;

    T* items;
    usize len;
    
    constexpr Slice() : items(nullptr), len(0) {}
    
    template<typename U>
    requires(ConvertibleTo<U, T>)
    constexpr Slice(const Slice<U>& slice) : items(slice.items), len(slice.len) {}

    template<usize N>
    constexpr Slice(T(&_items)[N])
        : items(_items), len(N)
    {}

    template<typename U, usize N>
    requires(ConvertibleTo<U, T>)
    constexpr Slice(U(&_items)[N])
        : items(_items), len(N)
    {}

    constexpr Slice(T* _items, const usize _len)
        : items(_items), len(_len)
    {}

    template<typename U>
    requires(ConvertibleTo<U, T>)
    constexpr Slice(U* _items, const usize _len)
        : items(_items), len(_len)
    {}

    template<typename U>
    requires(ConvertibleTo<U, T>)
    constexpr Slice& operator=(const Slice<U>& slice)
    {
        items = slice.items;
        len = slice.len;
        return *this;
    }

    [[nodiscard]] constexpr decltype(auto) operator[](this auto&& self, const usize index)
    {
        DebugAssert(index < self.len, "index out of range");
        return Forward<decltype(self)>(self).items[index];
    }

    [[nodiscard]] constexpr decltype(auto) begin(this auto&& self) { return self.items; };

    [[nodiscard]] constexpr decltype(auto) end(this auto&& self) { return self.items + self.len; };

    [[nodiscard]] constexpr decltype(auto) ptr(this auto&& self) { return self.items; };

    [[nodiscard]] constexpr MemoryAddress address() const { return reinterpret_cast<MemoryAddress>(ptr()); } 

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

    constexpr Slice<T> slice(usize count) const
    {
        DebugAssert(count <= len, "items out of range");
        return Slice<T>(items, count);
    }
};
