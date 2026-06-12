#pragma once
#include "collections/slice.h"


struct [[nodiscard]] StringView : Slice<const char>
{
    constexpr StringView() : Slice(nullptr, 0) {}
    
    constexpr StringView(const char* str, usize n) : Slice(str, n) {}
    
    template<usize N>
    constexpr StringView(const char(&str)[N]) : Slice(str, N-1) {}

    constexpr StringView(const Slice<char>& str) : Slice(str.items, str.len) {}
    
    constexpr StringView(const Slice<const char>& str) : Slice(str.items, str.len) {}

    [[nodiscard]] constexpr const char* ptr() const { return items; }

    constexpr StringView add(const usize offset) const
    {
        DebugAssert(ptr() && ((len - offset) > 0 || (len - offset) <= len), "invalid offset");
        return StringView(items + offset, len - offset);
    }

    constexpr StringView sub(const usize offset) const
    {
        DebugAssert(ptr() && ((len + offset) >= len), "invalid offset");
        return StringView(items - offset, len + offset);
    }
    
    [[nodiscard]] constexpr bool equals(const StringView& str) const;
    [[nodiscard]] constexpr bool ends_with(const StringView& str) const;
};

#include "mem/utils.h"


[[nodiscard]] constexpr bool StringView::equals(const StringView& str) const
{
    if (items == nullptr || len == 0)
    {
        return false;
    }
    return Mem::compare(Slice(items, str.len), str);
}
    
[[nodiscard]] constexpr bool StringView::ends_with(const StringView& str) const
{
    if(len < str.len)
    {
        return false;
    }
    
    if(Mem::compare(Slice(items + (len - str.len), str.len), str))
    {
        return true;
    }
    
    return false;
}