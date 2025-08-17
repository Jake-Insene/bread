#include "collections/string.h"


struct _BuffString
{
    // Enough for signed 64 bits numbers
    static constexpr usize BufferSize = 21;
    u8 buffer[BufferSize];
    usize buffer_index;

    constexpr u8* end() { return buffer + BufferSize; }
    constexpr StringView get_view() const
    {
        return StringView(
            (char*)buffer + (BufferSize - buffer_index), 
            buffer_index
        );
    }
};

template<typename T>
static constexpr void _integer_to_buff(_BuffString& buff, T integer)
{
    auto end = buff.end();

    u64 u = integer < 0 ? u64(-integer) : u64(integer);
    do
    {
        *--end = ('0' + u % 10);
        u /= 10;
        buff.buffer_index++;
    } while (u != 0);

    if constexpr (IsSigned<T>)
    {
        if (integer < 0)
        {
            *--end = '-';
            buff.buffer_index++;
        }
    }
}


String String::with_allocator(mem::Allocator allocator)
{
    return String
    {
        .allocator = allocator,
        .chars = {},
        .count = 0,
    };
}

String String::with_size(mem::Allocator allocator, usize size)
{
    return String
    {
        .allocator = allocator,
        .chars = mem::from_bytes<char>(allocator.alloc(size, alignof(usize))),
        .count = 0,
    };
 }

String String::from_chars(mem::Allocator allocator, StringView chars)
{
    String s = String::with_size(allocator, chars.len);
    
    if(chars.len != 0)
    {
        mem::copy(s.chars, chars);
        s.count = chars.len;
    }
    
    return s;
}

void String::destroy()
{
    if(chars.ptr())
    {
        allocator.free(mem::to_bytes(chars));
    }
}

void String::set(StringView new_chars)
{
    resize(new_chars.len);
    mem::copy(chars, new_chars);
    count = new_chars.len;
}

void String::resize(usize new_size)
{
    if (chars.len >= new_size)
    {
        count = new_size;
        return;
    }

    if (!chars.ptr())
    {
        chars = mem::from_bytes<char>(allocator.alloc(new_size, alignof(usize)));
        return;
    }

    if (!allocator.realloc(mem::to_bytes(chars), new_size, alignof(usize)))
    {
        auto new_chars = mem::from_bytes<char>(allocator.alloc(new_size, alignof(usize)));
        mem::copy(new_chars, chars);
        allocator.free(mem::to_bytes(chars));
        chars = new_chars;
    }
    else
    {
        chars.len = new_size;
        allocator.construct_array(chars.add(count));
    }

    count = new_size;
}

void String::add(StringView str)
{
    usize old_count = count;
    resize(count + str.len);
    mem::copy(chars.add(old_count), str);
}

bool String::equals(StringView str) const
{
    return StringView(chars.ptr(), count).equals(str);
}

bool String::ends_with(StringView str) const
{
    return StringView(chars.ptr(), count).ends_with(str);
}

StringView String::view()
{
    return StringView{chars.ptr(), count};
}

void String::_add_from_signed(i64 integer)
{
    _BuffString fmt_str = {};
    _integer_to_buff(fmt_str, integer);
    add(fmt_str.get_view());
}

void String::_add_from_unsigned(u64 integer)
{
    _BuffString fmt_str = {};
    _integer_to_buff(fmt_str, integer);
    add(fmt_str.get_view());
}

void String::_set_from_signed(i64 integer)
{
    _BuffString fmt_str = {};
    _integer_to_buff(fmt_str, integer);
    set(fmt_str.get_view());
}

void String::_set_from_unsigned(u64 integer)
{
    _BuffString fmt_str = {};
    _integer_to_buff(fmt_str, integer);
    set(fmt_str.get_view());
}
