#pragma once
#include "mem/allocator.h"
#include "mem/utils.h"

struct StringView;

struct [[nodiscard]] String
{
    mem::Allocator allocator{};
    Slice<char> chars{};
    usize count = 0;
    
    static String with_allocator(mem::Allocator allocator);
    
    static String with_size(mem::Allocator allocator, usize size);
    
    static String from_chars(mem::Allocator allocator, StringView chars);
    
    void destroy();
    
    void set(StringView new_chars);
    void resize(usize new_size);

    void add(StringView str);

    [[nodiscard]] bool equals(StringView str) const;
    [[nodiscard]] bool ends_with(StringView str) const;
    
    StringView view();
};
