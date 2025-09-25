#pragma once
#include "mem/allocator.h"
#include "mem/utils.h"


struct StringView;

namespace io
{
struct Writer;
}


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

    void resize(usize new_size);

    template<typename T>
    void set(const T arg)
    {
        if constexpr (IsSame<RemoveReference<T>, StringView>)
        {
            _set_str_view(arg);
        }
        else if constexpr (IsSame<RemoveConstPointer<T>, char>)
        {
            _set_str_view(StringView(arg, __string_len(arg)));
        }
        else if constexpr (IsInteger<T> && IsSigned<T>)
        {
            _set_from_signed(arg);
        }
        else if constexpr (IsInteger<T> && IsUnsigned<T>)
        {
            _set_from_unsigned(arg);
        }
        else
        {
            static_assert(false, "unknown argument type");
        }
    }

    template<typename T>
    void add(const T arg)
    {
        if constexpr (IsSame<RemoveReference<T>, StringView>)
        {
            _add_str_view(arg);
        }
        else if constexpr (IsSame<RemoveConstPointer<T>, char>)
        {
            _add_str_view(StringView(arg, __string_len(arg)));
        }
        else if constexpr (IsInteger<T> && IsSigned<T>)
        {
            _add_from_signed(arg);
        }
        else if constexpr (IsInteger<T> && IsUnsigned<T>)
        {
            _add_from_unsigned(arg);
        }
        else
        {
            static_assert(false, "unknown argument type");
        }
    }

    [[nodiscard]] bool equals(StringView str) const;
    [[nodiscard]] bool ends_with(StringView str) const;
    
    StringView view();

    io::Writer writer();

    void _set_str_view(StringView str);
    void _set_from_signed(i64 integer);
    void _set_from_unsigned(u64 integer);

    void _add_str_view(StringView str);
    void _add_from_signed(i64 integer);
    void _add_from_unsigned(u64 integer);
};

