#pragma once
#include "collections/string_view.h"
#include "core/templates.h"
#include "mem/allocator.h"


struct StringView;

namespace IO
{
struct Writer;
}


struct [[nodiscard]] String
{
    DisableCopy(String);
    DisableMove(String);

    static constexpr usize DefaultCapacity = 16;

    Mem::Allocator& allocator;
    Slice<char> chars;
    usize count;

    static String with_allocator(Mem::Allocator& allocator)
    {
        return String(allocator, 0, {});
    }

    static String from_chars(Mem::Allocator& allocator, StringView chars)
    {
        return String(allocator, 0, chars);
    }
    
    String(Mem::Allocator& allocator, usize initial_size, StringView initial_content);
    ~String();
    
    [[nodiscard]] char get(usize index) const
    {
        DebugAssert(index < count, "index out of range");
        return chars[index];
    }

    void resize(usize new_size);

    template<typename T>
    void set(T&& arg)
    {
        using Type = RemoveCVRef<T>;
        if constexpr (IsSame<Type, StringView>)
        {
            _set_str_view(arg);
        }
        else if constexpr (IsArrayOf<T, char>)
        {
            _set_str_view(arg);
        }
        else if constexpr (IsInteger<Type> && IsSigned<Type>)
        {
            _set_from_signed(arg);
        }
        else if constexpr (IsInteger<Type> && IsUnsigned<Type>)
        {
            _set_from_unsigned(arg);
        }
        else if constexpr (IsFloatingPoint<Type>)
        {
            _set_from_float(arg);
        }
        else
        {
            static_assert(false, "unknown argument type");
        }
    }

    template<typename T>
    void add(T&& arg)
    {
        using TypeNoCR = RemoveConst<RemoveReference<T>>;
        if constexpr (IsSame<TypeNoCR, StringView> || IsArrayOf<T, char>)
        {
            _add_str_view(arg);
        }
        else if constexpr (IsInteger<TypeNoCR> && IsSigned<TypeNoCR>)
        {
            _add_from_signed(arg);
        }
        else if constexpr (IsInteger<TypeNoCR> && IsUnsigned<TypeNoCR>)
        {
            _add_from_unsigned(arg);
        }
        else if constexpr (IsFloatingPoint<TypeNoCR>)
        {
            _add_from_float(arg);
        }
        else
        {
            static_assert(false, "unknown argument type");
        }
    }

    [[nodiscard]] bool equals(StringView str) const;
    [[nodiscard]] bool ends_with(StringView str) const;
    
    StringView view() const;

    IO::Writer writer();

    void _set_str_view(StringView str);
    void _set_from_signed(i64 integer);
    void _set_from_unsigned(u64 integer);
    void _set_from_float(f64 fp);

    void _add_str_view(StringView str);
    void _add_from_signed(i64 integer);
    void _add_from_unsigned(u64 integer);
    void _add_from_float(f64 fp);
};

