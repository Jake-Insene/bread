#include "mem/generic_allocator.h"

#include "mem/utils.h"


namespace mem
{
    
    static inline GenericAllocator::Header* get_header(Slice<u8> ptr)
    {
        return (GenericAllocator::Header*)(ptr.sub(sizeof(GenericAllocator::Header)).ptr());
    }
    
    void GenericAllocator::destroy()
    {
#if DEBUG
        Log::info("Allocated pages %llu", page_count);
#endif
        for(usize i = 0; i < page_count; i++)
        {
            Page& page = allocated_pages[i];
#if DEBUG
            Header* header = page.first_header;
            usize header_count = 0;
            while(header)
            {
                header_count++;
                DebugAssert((header->tags & Allocated) == 0, "Forget to call free.");
                header = header->next;
            }
            Log::info("Page at address %p of size %llu, with %llu headers", page.bytes.ptr(), page.bytes.len, header_count);
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
        page.bytes = internal_allocator.alloc(size, mem::get_page_size());
        page.first_header = nullptr;
        page.last_header = nullptr;
#if !defined(NDEBUG)
        Log::info("Page allocated at %p with size %llu", page.bytes.ptr(), page.bytes.len);
#endif
        return page;
    }
    
    Slice<u8> GenericAllocator::alloc(usize size, usize alignment)
    {
        DebugAssert(alignment == mem::align_up<usize>(alignment, 2), "Alignment must be a power of 2");

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
        constexpr usize MinimumValidRemain = mem::align_up(sizeof(Header) * 2, alignof(Header));
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

                    const usize required_size = mem::align_up(aligned_size + sizeof(Header), alignment);
                    if(allocated_mem->len >= required_size)
                    {
                        allocated_mem->tags |= Allocated;

                        u8* aligned_base = (u8*)mem::align_up(usize(allocated_mem) + sizeof(Header), alignment);
                        isize offset = aligned_base - ((u8*)allocated_mem + sizeof(Header));

                        if (offset > 0)
                        {
                            if (allocated_mem->len >= aligned_size + offset)
                            {
                                const Header copied_block = *allocated_mem;
                                Header* prev = allocated_mem->prev;

                                allocated_mem = (Header*)((u8*)allocated_mem + offset);
                                *allocated_mem = copied_block;
                                allocated_mem->len -= offset;

                                prev->next = allocated_mem;
                                if(allocated_mem->next)
                                {
                                    allocated_mem->next->prev = allocated_mem;
                                }
                            }
                            else
                            {
                                // Try it with the next block
                                allocated_mem = allocated_mem->next;
                                continue;
                            }
                        }

                        const usize remain = allocated_mem->len - aligned_size;

                        allocated_mem->len = aligned_size;
                        page.last_header = allocated_mem;

                        if (remain >= MinimumValidRemain)
                        {
                            u8* remain_base = (u8*)(usize(aligned_base) + aligned_size);

                            Header* remain_header = (Header*)remain_base;
                            allocated_mem->next = remain_header;

                            remain_header->len = remain - sizeof(Header);
                            remain_header->page_index = allocated_mem->page_index;
                            remain_header->tags = 0;
                            remain_header->prev = allocated_mem;
                            remain_header->next = nullptr;

                            page.last_header = remain_header;
                        }

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

        Page& new_page = allocate_new_page(aligned_size + sizeof(Header));
        u8* base = new_page.bytes.ptr();
        u8* aligned_mem = (u8*)mem::align_up<usize>(usize(base) + sizeof(Header), alignment);

        Header* allocation_header = (Header*)(aligned_mem - sizeof(Header));
        allocation_header->len = new_page.bytes.len - sizeof(Header);
        allocation_header->page_index = page_count-1;
        allocation_header->tags = Allocated;
        allocation_header->prev = nullptr;
        allocation_header->next = nullptr;
        
        new_page.first_header = allocation_header;
        new_page.last_header = allocation_header;

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
                new_page.last_header = fill_header;
            }
        }
        
        return Slice<u8>
        {
            aligned_mem,
            size,
        };
    }
        
    bool GenericAllocator::realloc(Slice<u8> ptr, usize new_size, usize alignment)
    {
        DebugAssert(alignment == mem::align_up<usize>(alignment, 2), "Alignment must be a power of 2");
        DebugAssert(ptr.ptr(), "Invalid pointer");
        Header* header = get_header(ptr);
        DebugAssert(header->tags & Allocated, "The given block is already free.");

        if(header->len >= new_size)
        {
            return true;
        }

        return false;
    }
        
    void GenericAllocator::free(Slice<u8> ptr)
    {
        DebugAssert(ptr.ptr(), "Invalid pointer");
        Header* header = get_header(ptr);
        DebugAssert(ptr.ptr() && (header->tags & Allocated), "The given block is already free.");

        header->tags = None;
        if (header->prev && (header->prev->tags & Allocated) == 0)
        {
            Header* prev = header->prev;
            prev->len += header->len + sizeof(Header);
            prev->next = header->next;
            if(prev->next)
            {
                prev->next->prev = prev;
            }
        }
        else if (header->next && (header->next->tags & Allocated) == 0)
        {
            Header* next = header->next;
            header->len += next->len + sizeof(Header);
            header->next = next->next;
            if (header->next)
            {
                header->next->prev = header;
            }
        }
    }
        
    Allocator GenericAllocator::allocator()
    {
        return Allocator
        {
            .vtable = 
            {
                .alloc = (decltype(Allocator::VTable::alloc))&GenericAllocator::alloc,
                .realloc = (decltype(Allocator::VTable::realloc))&GenericAllocator::realloc,
                .free = (decltype(Allocator::VTable::free))&GenericAllocator::free,
            },
            .self = (Allocator*)this,
        };
    }
    
}