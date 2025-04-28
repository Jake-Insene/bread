#include "core/string.h"


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
        .chars = mem::from_bytes<char>(allocator.alloc(size, alignof(char*))),
        .count = 0,
    };
 }

String String::from_chars(mem::Allocator allocator, StringView chars)
{
    String s = String::with_size(allocator, chars.len);
    
    if(chars.len != 0)
    {
        std::memcpy(s.chars.ptr(), chars.ptr(), chars.len);
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
