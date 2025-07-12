#pragma once
#include "mem/allocator.h"
#include "mem/utils.h"

struct StringView;


struct [[nodiscard]] String
{
    mem::Allocator allocator;
    Slice<char> chars;
    usize count;
    
    static String with_allocator(mem::Allocator allocator);
    
    static String with_size(mem::Allocator allocator, usize size);
    
    static String from_chars(mem::Allocator allocator, StringView chars);
    
    void destroy();
    
    [[nodiscard]] char get(usize index) const
    {
        return chars[index];
    }

    void set(StringView new_chars);
    void resize(usize new_size);

    void add(StringView str);

    // Conversion
    template<typename T>
    void set_from_integer(T integer)
    {
        static_assert(IsInteger<T> == true, "an integer type was expected");
        if constexpr (IsSigned<T>)
        {
            _set_from_signed(integer);
        }
        else
        {
            _set_from_unsigned(integer);
        }
    }

    [[nodiscard]] bool equals(StringView str) const;
    [[nodiscard]] bool ends_with(StringView str) const;
    
    StringView view();

    void _set_from_signed(i64 integer);
    void _set_from_unsigned(u64 integer);
};
