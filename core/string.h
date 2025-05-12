#pragma once
#include "core/slice.h"
#include "mem/allocator.h"
#include "mem/utils.h"

#include <memory>

inline constexpr usize stringlen(const char* str)
{
    usize len = 0;
    while(*str++)
            len++;
            
    return len;
}

struct [[nodiscard]] StringView : Slice<const char>
{
    constexpr StringView() : Slice(0, 0) {}
    
    constexpr StringView(const char* str, usize n) : Slice(str, n) {}
    
    template<usize N>
    constexpr StringView(const char(&str)[N]) : Slice(str, N-1) {}
    
    constexpr StringView(Slice<char> str) : Slice(str.items, str.len) {}

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
    
    [[nodiscard]] constexpr bool equals(StringView str) const
    {
        return mem::compare(Slice(items, str.len), str);
    }
    
    [[nodiscard]] constexpr bool ends_with(StringView str) const
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
};


struct [[nodiscard]] String
{
    mem::Allocator allocator{};
    Slice<char> chars{};
    usize count = 0;
    
    static String with_allocator(mem::Allocator allocator);
    
    static String with_size(mem::Allocator allocator, usize size);
    
    static String from_chars(mem::Allocator allocator, StringView chars);
    
    void destroy();
    
    [[nodiscard]] bool equals(StringView str) const;
    [[nodiscard]] bool ends_with(StringView str) const;
    
    StringView view();
};
