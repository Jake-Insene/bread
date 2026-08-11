#pragma once
#include "Core/Header.h"
#include "debug/assertion.h"


/**
* Contains a linear collection of T elements.
* Read and Write are allowed.
*
* @tparam T The type of the elements in the slice.
*/
template<typename T>
struct [[nodiscard]] Slice
{
    using Type = T;

    T* items;
    usize len;
    
    constexpr Slice() : items(nullptr), len(0) {}
    
    template<usize N>
    constexpr Slice(T(&_items)[N])
        : items(_items), len(N)
    {}

    constexpr Slice(T* _items, const usize _len)
        : items(_items), len(_len)
    {}

    [[nodiscard]] constexpr decltype(auto) operator[](this auto&& self, const usize index)
    {
        DebugAssert(index < self.len, "index out of range");
        return Core::Forward<decltype(self)>(self).items[index];
    }

    [[nodiscard]] constexpr decltype(auto) begin(this auto&& self) { return self.items; };

    [[nodiscard]] constexpr decltype(auto) end(this auto&& self) { return self.items + self.len; };

    [[nodiscard]] constexpr decltype(auto) ptr(this auto&& self) { return self.items; };

    [[nodiscard]] constexpr MemoryAddress address() const { return reinterpret_cast<MemoryAddress>(ptr()); } 

    [[nodiscard]] constexpr bool null() const { return items == nullptr; }

    constexpr Slice add(const usize offset) const
    {
        DebugAssert(ptr() && ((len - offset) > 0 || (len - offset) <= len), "invalid offset");
        return Slice(items + offset, len - offset);
    }

    constexpr Slice sub(const usize offset) const
    {
        DebugAssert(ptr() && ((len + offset) >= len), "invalid offset");
        return Slice(items - offset, len + offset);
    }

    constexpr Slice slice(usize count) const
    {
        DebugAssert(count <= len, "items out of range");
        return Slice(items, count);
    }

    constexpr Slice<const Type> as_const() const
    {
        return Slice<const Type>(items, len);
    }
};
