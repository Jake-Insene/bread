#pragma once
#include "collections/slice.h"

inline constexpr usize __string_len(const char* str)
{
    usize len = 0;
    while(*str++)
            len++;
            
    return len;
}

struct [[nodiscard]] StringView : Slice<const char>
{
    constexpr StringView() : Slice(nullptr, 0) {}
    
    constexpr StringView(const char* str, usize n) : Slice(str, n) {}
    
    template<usize N>
    constexpr StringView(const char(&str)[N]) : Slice(str, N-1) {}

    constexpr StringView(Slice<char> str) : Slice(str.items, str.len) {}
    
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
    
    [[nodiscard]] constexpr bool equals(StringView str) const;
    [[nodiscard]] constexpr bool ends_with(StringView str) const;
};

#include "mem/utils.h"


[[nodiscard]] constexpr bool StringView::equals(StringView str) const
{
    if (items == nullptr || len == 0)
        return false;
    return mem::compare(Slice(items, str.len), str);
}
    
[[nodiscard]] constexpr bool StringView::ends_with(StringView str) const
{
    if(len < str.len)
    {
        return false;
    }
    
    if(mem::compare(Slice<const char>(items + (len - str.len), str.len), str))
    {
        return true;
    }
    
    return false;
}