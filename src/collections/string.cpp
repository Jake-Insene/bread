#include "collections/string.h"


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
        return;

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
}

void String::add(StringView str)
{
    resize(count + str.len);
    mem::copy(chars.add(count), str);
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

void String::_set_from_signed(i64 integer)
{
    // Enough for signed 64 bits numbers
    static constexpr usize BufferSize = 21;
    u8 buffer_storage[BufferSize] = {};
    auto end = buffer_storage + BufferSize;
    usize buffer_index = 0;

    u64 u = integer < 0 ? u64(-integer) : u64(integer);
    do
    {
        *--end = ('0' + u % 10);
        u /= 10;
        buffer_index++;
    } while (u != 0);

    if (integer < 0)
    {
        *--end = '-';
        buffer_index++;
    }

    set(StringView((char*)buffer_storage + (BufferSize - buffer_index), buffer_index));
}

void String::_set_from_unsigned(u64 integer)
{
    // Enough for unsigned 64 bits numbers
    static constexpr usize BufferSize = 21;
    u8 buffer_storage[BufferSize] = {};
    auto end = buffer_storage + BufferSize;
    usize buffer_index = 0;

    u64 u = integer < 0 ? u64(-integer) : u64(integer);
    do
    {
        *--end = ('0' + u % 10);
        u /= 10;
        buffer_index++;
    } while (u != 0);
 
    set(StringView((char*)buffer_storage, buffer_index));
}
