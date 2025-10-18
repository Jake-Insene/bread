#include "collections/string.h"
#include "collections/string_view.h"
#include "collections/string_utility.h"


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
        count = new_size;
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

io::Writer String::writer()
{
    io::Writer writer = {};
    writer.self = this;
    writer.write_fn = [](Opaque self, const Slice<const u8> bytes) -> void
        {
            String* str = self.cast<String*>();
            str->add(StringView((const char*)bytes.ptr(), bytes.len));
        };
    return writer;
}

void String::_set_str_view(StringView str)
{
    resize(str.len);
    mem::copy(chars, str);
    count = str.len;
}

void String::_set_from_signed(i64 integer)
{
    StringResult result = StringUtility::integer_to_string(integer, 10);
    set(StringView((char*)(result.result + result.begin), result.len));
}

void String::_set_from_unsigned(u64 integer)
{
    StringResult result = StringUtility::integer_to_string(integer, 10);
    set(StringView((char*)(result.result + result.begin), result.len));
}

void String::_add_str_view(StringView str)
{
    usize old_count = count;
    resize(count + str.len);
    mem::copy(chars.add(old_count), str);
}

void String::_add_from_signed(i64 integer)
{
    StringResult result = StringUtility::integer_to_string(integer, 10);
    add(StringView((char*)(result.result + result.begin), result.len));
}

void String::_add_from_unsigned(u64 integer)
{
    StringResult result = StringUtility::integer_to_string(integer, 10);
    add(StringView((char*)(result.result + result.begin), result.len));
}

