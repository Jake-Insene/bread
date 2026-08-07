#include "collections/string.h"

#include "collections/string_view.h"
#include "collections/string_utility.h"
#include "io/writer.h"
#include "math/funcs.h"
#include "mem/utils.h"


String::String(Mem::Allocator& allocator, usize initial_size, StringView initial_content)
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

String::~String()
{
    if(chars.ptr())
    {
        allocator.free(Mem::to_bytes(chars));
        chars = {};
    }
}

void String::resize(usize new_size)
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
        Slice chars_to_construct = chars.add(count);
        ConstructArray(chars_to_construct.ptr(), chars_to_construct.len);
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

StringView String::view() const
{
    return StringView(chars.ptr(), count);
}

IO::Writer String::writer()
{
    IO::Writer writer = {};
    writer.writable = reinterpret_cast<Opaque*>(this);
    writer.write_fn = [](Opaque* self, const Slice<const u8>& bytes) -> void
    {
        String& str = *self->cast<String*>();
        str.add(StringView(reinterpret_cast<const char*>(bytes.ptr()), bytes.len));
    };
    return writer;
}

void String::_set_str_view(StringView str)
{
    resize(str.len);
    Mem::copy(chars, str);
    count = str.len;
}

void String::_set_from_signed(i64 integer)
{
    StringResult result = StringUtility::integer_to_string(integer, 10);
    set(StringView(reinterpret_cast<char*>(result.result + result.begin), result.len));
}

void String::_set_from_unsigned(u64 integer)
{
    StringResult result = StringUtility::integer_to_string(integer, 10);
    set(StringView(reinterpret_cast<char*>(result.result + result.begin), result.len));
}

void String::_set_from_float(f64 fp)
{
    StringResult result = StringUtility::fp_to_string(fp, 2);
    set(StringView(reinterpret_cast<char*>(result.result + result.begin), result.len));
}

void String::_add_str_view(StringView str)
{
    if (str.len == 0)
    {
        return;
    }

    usize old_count = count;
    resize(count + str.len);
    Mem::copy(chars.add(old_count), str);
}

void String::_add_from_signed(i64 integer)
{
    StringResult result = StringUtility::integer_to_string(integer, 10);
    add(StringView(reinterpret_cast<char*>(result.result + result.begin), result.len));
}

void String::_add_from_unsigned(u64 integer)
{
    StringResult result = StringUtility::integer_to_string(integer, 10);
    add(StringView(reinterpret_cast<char*>(result.result + result.begin), result.len));
}

void String::_add_from_float(f64 fp)
{
    StringResult result = StringUtility::fp_to_string(fp, 2);
    add(StringView(reinterpret_cast<char*>(result.result + result.begin), result.len));
}

