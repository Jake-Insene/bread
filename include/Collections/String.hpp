#pragma once
#include "Core/Header.hpp"
#include "Collections/StringView.hpp"
#include "fmt/fmt.h"
#include "IO/Writer.hpp"
#include "math/funcs.h"
#include "Mem/Allocator.hpp"


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
    
    String(Mem::Allocator& allocator, usize initial_size, StringView initial_content)
    : allocator(allocator), chars(), count()
    {
        usize initial_capacity = initial_size == 0 ? DefaultCapacity : initial_size;
        if(initial_size == 0)
        {
            initial_capacity = Math::min(DefaultCapacity, initial_content.len);
        }

        chars = Mem::from_bytes<char>(
            allocator.alloc(sizeof(char) * initial_capacity, alignof(char))
        );
        count = 0;

        if(!initial_content.null())
        {
            Mem::copy(chars, initial_content);
            count = initial_content.len;
        }
    }

    ~String()
    {
        if(chars.ptr())
        {
            allocator.free(Mem::to_bytes(chars));
            chars = {};
        }
    }
    
    [[nodiscard]] char get(usize index) const
    {
        DebugAssert(index < count, "index out of range");
        return chars[index];
    }

    void resize(usize new_size)
    {
        if (chars.len >= new_size)
        {
            count = new_size;
            return;
        }

        if (chars.null())
        {
            chars = Mem::from_bytes<char>(allocator.alloc(new_size, alignof(usize)));
            count = new_size;
            return;
        }

        if (!allocator.realloc(Mem::to_bytes(chars), new_size, alignof(usize)))
        {
            Slice new_chars = Mem::from_bytes<char>(allocator.alloc(new_size, alignof(usize)));
            Mem::copy(new_chars, chars);
            allocator.free(Mem::to_bytes(chars));
            chars = new_chars;
        }
        else
        {
            chars.len = new_size;
            Slice chars_to_fill = chars.add(count);
            Mem::zero(chars_to_fill);
        }

        count = new_size;
    }

    template<typename T>
    void set(T&& arg)
    {
        using TypeNoCR = Core::RemoveConst<Core::RemoveReference<T>>;
        if constexpr(Core::IsSame<TypeNoCR, StringView>)
        {
            _set_str_view(arg);
        }
        else if constexpr(Core::IsArrayOf<T, char>)
        {
            _set_str_view(arg);
        }
        else if constexpr(Core::IsInteger<TypeNoCR>)
        {
            StringResult result = StringUtility::integer_to_string<TypeNoCR>(arg, 10);
            _set_str_view(StringView(reinterpret_cast<char*>(result.result + result.begin), result.len));
        }
        else if constexpr(Core::IsFloatingPoint<TypeNoCR>)
        {
            StringResult result = StringUtility::fp_to_string<TypeNoCR>(arg, 2);
            _set_str_view(StringView(reinterpret_cast<char*>(result.result + result.begin), result.len));
        }
        else
        {
            static_assert(false, "unknown argument type");
        }
    }

    template<typename T>
    void add(T&& arg)
    {
        using TypeNoCR = Core::RemoveConst<Core::RemoveReference<T>>;
        if constexpr(Core::IsSame<TypeNoCR, StringView> || Core::IsArrayOf<T, char>)
        {
            _add_str_view(arg);
        }
        else if constexpr(Core::IsInteger<TypeNoCR>)
        {
            StringResult result = StringUtility::integer_to_string<TypeNoCR>(arg, 10);
            _add_str_view(StringView(reinterpret_cast<char*>(result.result + result.begin), result.len));
        }
        else if constexpr(Core::IsFloatingPoint<TypeNoCR>)
        {
            StringResult result = StringUtility::fp_to_string<TypeNoCR>(arg, 2);
            _add_str_view(StringView(reinterpret_cast<char*>(result.result + result.begin), result.len));
        }
        else
        {
            static_assert(false, "unknown argument type");
        }
    }

    [[nodiscard]] bool equals(StringView str) const
    {
        return view().equals(str);
    }

    [[nodiscard]] bool ends_with(StringView str) const
    {
        return view().ends_with(str);
    }
    
    StringView view() const
    {
        return StringView(chars.ptr(), count);
    }

    IO::Writer writer()
    {
        IO::Writer writer = {};
        writer.writable = reinterpret_cast<Core::Opaque*>(this);
        writer.write_fn = [](Core::Opaque* self, const Slice<const u8>& bytes) -> void
        {
            String& str = *self->cast<String*>();
            str.add(StringView(reinterpret_cast<const char*>(bytes.ptr()), bytes.len));
        };
        return writer;
    }

    void _set_str_view(StringView str)
    {
        resize(str.len);
        Mem::copy(chars, str);
        count = str.len;
    }

    void _add_str_view(StringView str)
    {
        if (str.len == 0)
        {
            return;
        }

        usize old_count = count;
        resize(count + str.len);
        Mem::copy(chars.add(old_count), str);
    }
};

namespace Format
{

template<>
struct Formatter<String>
{
	static void format_custom(const IO::Writer& writer, const String& str)
    {
        format<false>(writer, "{}", str.view());
    }
};

}
