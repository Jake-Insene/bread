#include "mem/generic_allocator.h"

#include "debug/fail.h"
#include "log/log.h"
#include "mem/utils.h"
#include "os/os.h"


namespace mem
{
    
static inline GenericAllocator::Header* get_header(Slice<u8> ptr)
{
    return reinterpret_cast<GenericAllocator::Header*>(ptr.sub(sizeof(GenericAllocator::Header)).ptr());
}
    
void GenericAllocator::destroy()
{
    Log::debug("[Memory]: Allocated pages {}", page_count);
  
    for(usize i = 0; i < page_count; i++)
    {
        Page& page = allocated_pages[i];
#if defined(DEBUG)
        usize page_size_accumulator = 0;

        Header* header = page.first_header;
        usize header_count = 0;
        while(header)
        {
            page_size_accumulator += header->len + sizeof(Header);
            header_count++;
            DebugAssert((header->tags & Allocated) == 0, "forget to call free.");
            header = header->next;
        }

        DebugAssert(page_size_accumulator == page.bytes.len, "allocator corruption detected");
        Log::debug("[Memory]: Page at address {} of size {}, with {} headers", page.bytes.ptr(), page.bytes.len, header_count);
#endif
        internal_allocator.free(page.bytes);
    }
    
    if(allocated_pages.ptr())
    {
        internal_allocator.free(mem::to_bytes(allocated_pages));
    }
}

GenericAllocator::Page& GenericAllocator::allocate_new_page(usize size)
{
    Page& page = allocated_pages[page_count++];
    page.bytes = internal_allocator.alloc(size, OS::get_page_size());
    page.first_header = nullptr;
    Log::debug("[Memory]: Page requested at address {} with size {}", page.bytes.ptr(), page.bytes.len);
    return page;
}

void GenericAllocator::check_integrity()
{
#if DEBUG
    for (usize i = 0; i < page_count; i++)
    {
        usize page_size_accumulator = 0;
        Page& page = allocated_pages[i];
        Header* header = page.first_header;
        while (header)
        {
            page_size_accumulator += header->len + sizeof(Header);
            header = header->next;
        }

        DebugAssert(page_size_accumulator == page.bytes.len, "allocator corruption");
    }
#endif
}
    
Slice<u8> GenericAllocator::alloc(usize size, usize alignment)
{
    DebugAssert(alignment == mem::align_up<usize>(alignment, 2), "alignment must be a power of 2");

    if(allocated_pages.null())
    {
        page_count = 0;
        allocated_pages = mem::from_bytes<Page>(internal_allocator.alloc(sizeof(Page) * DefaultPageListSize, alignof(Page)));
    }
    else if(page_count >= allocated_pages.len)
    {
        FailOn(allocated_pages.len >= MaxPageCount, "Allocator reaches its limit!");
        
        usize new_size = allocated_pages.len + allocated_pages.len / 2;
        if(internal_allocator.realloc(mem::to_bytes(allocated_pages), sizeof(Page) * new_size, alignof(Page)))
        {
            allocated_pages.len = new_size;
        }
        else
        {
            Slice<Page> new_pages = mem::from_bytes<Page>(internal_allocator.alloc(sizeof(Page) * new_size, alignof(Page)));
            mem::copy(new_pages, allocated_pages);
            internal_allocator.free(mem::to_bytes(allocated_pages));
            allocated_pages = new_pages;
        }
    }

    const usize aligned_size = mem::align_up(size, alignment);
    for(usize i = 0; i < page_count; i++)
    {
        Page& page = allocated_pages[i];

        if(page.first_header != nullptr)
        {
            Header* allocated_mem = page.first_header;
            while(allocated_mem != nullptr)
            {
                if (allocated_mem->tags & Allocated)
                {
                    allocated_mem = allocated_mem->next;
                    continue;
                }

                if(allocated_mem->len >= aligned_size)
                {
                    u8* aligned_base = (u8*)mem::align_up(usize(allocated_mem) + sizeof(Header), alignment);
                    isize offset = aligned_base - ((u8*)allocated_mem + sizeof(Header));

                    if (offset > 0)
                    {
                        if (allocated_mem->len >= aligned_size + offset)
                        {
                            const Header copied_block = *allocated_mem;
                            Header* prev = allocated_mem->prev;

                            allocated_mem = reinterpret_cast<Header*>(aligned_base - sizeof(Header));
                            allocated_mem->len = copied_block.len - offset;
                            allocated_mem->page_index = i;
                            allocated_mem->tags = 0;

                            // Is impossible that a header that start at 0xXXXX'X000 it is not aligned correctly.
                            prev->len += offset;
                            prev->next = allocated_mem;

                            allocated_mem->prev = copied_block.prev;
                            allocated_mem->next = copied_block.next;
                            if (allocated_mem->next)
                                allocated_mem->next->prev = allocated_mem;

                            check_integrity();
                        }
                        else
                        {
                            // Try it with the next block
                            allocated_mem = allocated_mem->next;
                            continue;
                        }
                    }

                    const usize remain = allocated_mem->len - aligned_size;
                    
                    if (remain >= MinimumValidRemain)
                    {
                        u8* remain_base = (u8*)(usize(aligned_base) + aligned_size);

                        Header* remain_header = reinterpret_cast<Header*>(remain_base);

                        remain_header->len = remain - sizeof(Header);
                        remain_header->page_index = allocated_mem->page_index;
                        remain_header->tags = 0;
                        remain_header->prev = allocated_mem;
                        remain_header->next = allocated_mem->next;

                        allocated_mem->len = aligned_size;
                        allocated_mem->next = remain_header;

                        if (remain_header->next)
                        {
                            remain_header->next->prev = remain_header;
                        }
                    }
                    
                    allocated_mem->tags |= Allocated;
                    check_integrity();

                    return Slice<u8>
                    {
                        aligned_base,
                        size
                    };
                }

                
                allocated_mem = allocated_mem->next;
            }
        }
    }

    Page& new_page = allocate_new_page(next_page_size + aligned_size + sizeof(Header));
    next_page_size += DefaultNextPageSize;

    u8* base = new_page.bytes.ptr();
    u8* aligned_mem = reinterpret_cast<u8*>(mem::align_up<usize>(usize(base) + sizeof(Header), alignment));

    Header* allocation_header = reinterpret_cast<Header*>(aligned_mem - sizeof(Header));
    allocation_header->len = new_page.bytes.len - sizeof(Header);
    allocation_header->page_index = page_count - 1;
    allocation_header->tags = Allocated;
    allocation_header->prev = nullptr;
    allocation_header->next = nullptr;
    
    new_page.first_header = allocation_header;

    // Trying to dividing the memory if the requested memory is too slow
    if(allocation_header->len > aligned_size)
    {
        usize unused_size = allocation_header->len - aligned_size;
        if(unused_size >= MinimumValidRemain)
        {
            // Creating new header
            Header* fill_header = (Header*)(aligned_mem + aligned_size);
            fill_header->len = unused_size - sizeof(Header);
            fill_header->page_index = allocation_header->page_index;
            fill_header->tags = 0;
            fill_header->prev = allocation_header;
            fill_header->next = nullptr;
            
            allocation_header->len = aligned_size;
            allocation_header->next = fill_header;
        }
    }
    
    check_integrity();

    return Slice<u8>
    {
        aligned_mem,
        size,
    };
}
        
bool GenericAllocator::realloc(Slice<u8> ptr, usize new_size, usize alignment)
{
    DebugAssert(alignment == mem::align_up<usize>(alignment, 2), "alignment must be a power of 2");
    DebugAssert(ptr.ptr(), "invalid pointer");

    Header* header = get_header(ptr);
    DebugAssert(header->tags & Allocated, "the given block is already free.");
    
    check_integrity();
    
    if(mem::align_up(new_size, alignment) <= header->len)
        return true;

    return false;
}
        
void GenericAllocator::free(Slice<u8> ptr)
{
    DebugAssert(ptr.ptr() != nullptr, "invalid pointer");
    
    Header* header = get_header(ptr);
    DebugAssert(header->tags & Allocated, "the given block is already free.");
    
    header->tags = None;

    if(header && header->prev && header->prev->tags == 0)
    {
        header->prev->len += header->len + sizeof(Header);
        header->prev->next = header->next;

        if (header->next)
            header->next->prev = header->prev;

        header = header->prev;
    }


    if(header && header->next && header->next->tags == 0)
    {
        header->len += header->next->len + sizeof(Header);
        header->next = header->next->next;
        
        if (header->next)
            header->next->prev = header;
        
        header = header->next;
    }

    check_integrity();
}
    
static inline Allocator::VTable ga_vtable = 
{
    .alloc = (decltype(Allocator::VTable::alloc))&GenericAllocator::alloc,
    .realloc = (decltype(Allocator::VTable::realloc))&GenericAllocator::realloc,
    .free = (decltype(Allocator::VTable::free))&GenericAllocator::free,
};
Allocator GenericAllocator::allocator()
{
    return Allocator
    {
        .vtable = &ga_vtable,
        .self = (Allocator*)this,
    };
}
    
}