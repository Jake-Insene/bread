#pragma once
#include "mem/page_allocator.h"

namespace mem
{
    
    struct GenericAllocator : Allocator
    {
        static constexpr u16 MaxPageCount = u16(-1);
        static constexpr usize DefaultPageListSize = 128;
        static constexpr usize DirectPageAllocationSize = 32 * 1024; // 256 KB

        enum HeaderTags
        {
            None = 0,
            Allocated,
        };
        
        struct Header
        {
            usize len;
            usize page_index : 16;
            usize tags : 48;
            Header* prev;
            Header* next;
        };
        static_assert(sizeof(Header) == 32);

        struct Page
        {
            Slice<u8> bytes;
            Header* first_header;
            Header* last_header;
        };

        PageAllocator internal_allocator;
        Slice<Page> allocated_pages;
        usize page_count = 0;

        void destroy();

        Page& allocate_new_page(usize size);
        
        Slice<u8> alloc(usize size, usize alignment);
        
        bool realloc(Slice<u8> ptr, usize new_size, usize alignment);
        
        void free(Slice<u8> ptr);
       
        Allocator allocator();
    };
    
}
