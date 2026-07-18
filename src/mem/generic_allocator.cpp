#include "mem/generic_allocator.h"

#include "debug/fail.h"
#include "debug/log.h"
#include "mem/utils.h"
#include "os/os.h"


namespace Mem
{
    
static inline GenericAllocator::Header* get_header(Slice<u8> ptr)
{
    return reinterpret_cast<GenericAllocator::Header*>(ptr.sub(sizeof(GenericAllocator::Header)).ptr());
}

void GenericAllocator::init()
{
    ConstructObject(internal_allocator);
    allocated_pages = {};
    page_count = 0;
    next_page_size = DefaultNextPageSize;
    index = 0;
}
    
void GenericAllocator::destroy()
{
    usize accumulator = 0;
    for(usize i = 0; i < page_count; i++)
    {
        Page& page = allocated_pages[i];
        accumulator += page.bytes.len;
    }
    
    Log::debug("[Memory]: Allocated pages {}, total memory usage of {} MiB",
        page_count, f32(accumulator) / MiB(1));
  
    for(usize i = 0; i < page_count; i++)
    {
        Page& page = allocated_pages[i];
#if defined(DEBUG)
        usize page_size_accumulator = 0;

        Header* header = page.first_header;
        usize header_count = 0;
        while(header != nullptr)
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
    
    if(!allocated_pages.null())
    {
        internal_allocator.free(Mem::to_bytes(allocated_pages));
    }

    page_count = 0;
}
    
Slice<u8> GenericAllocator::alloc(usize size, usize alignment)
{
    DebugAssert(alignment == 1 || alignment == Mem::align_up<usize>(alignment, 2), "alignment must be a power of 2 or 1");

    if(allocated_pages.null())
    {
        page_count = 0;
        allocated_pages = Mem::from_bytes<Page>(internal_allocator.alloc(sizeof(Page) * DefaultPageListSize, alignof(Page)));
    }
    else if(page_count >= allocated_pages.len)
    {
        FailOn(allocated_pages.len >= MaxPageCount, "allocator reaches its limit!");
        
        usize new_size = allocated_pages.len + (allocated_pages.len / 2);
        if(internal_allocator.realloc(Mem::to_bytes(allocated_pages), sizeof(Page) * new_size, alignof(Page)))
        {
            allocated_pages.len = new_size;
        }
        else
        {
            Slice new_pages = Mem::from_bytes<Page>(internal_allocator.alloc(sizeof(Page) * new_size, alignof(Page)));
            Mem::copy(new_pages, allocated_pages);
            internal_allocator.free(Mem::to_bytes(allocated_pages));
            allocated_pages = new_pages;
        }
    }

    const usize aligned_size = Mem::align_up(size, alignment);

    Header* allocated_mem = _search_for_available_space(aligned_size, alignment);
    if(allocated_mem != nullptr)
    {
        // allocated_mem = aligned_base - sizeof(Header)
        u8* base = reinterpret_cast<u8*>(usize(allocated_mem) + sizeof(Header));

        allocated_mem->tags |= Allocated;
        _check_integrity();

        index++;
        allocated_mem->index = index;
        return Slice
        {
            base,
            size
        };
    }

    Page& new_page = _allocate_new_page(next_page_size + aligned_size + sizeof(Header));
    next_page_size *= 2;

    u8* base = new_page.bytes.ptr();
    u8* aligned_mem = reinterpret_cast<u8*>(Mem::align_up<usize>(usize(base) + sizeof(Header), alignment));

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
            Header* fill_header = reinterpret_cast<Header*>(aligned_mem + aligned_size);
            fill_header->len = unused_size - sizeof(Header);
            fill_header->page_index = allocation_header->page_index;
            fill_header->tags = 0;
            fill_header->prev = allocation_header;
            fill_header->next = nullptr;
            
            allocation_header->len = aligned_size;
            allocation_header->next = fill_header;
        }
    }
    
    _check_integrity();

    index++;
    allocation_header->index = index;

    return Slice
    {
        aligned_mem,
        size,
    };
}
        
bool GenericAllocator::realloc(const Slice<u8>& ptr, usize new_size, usize alignment)
{
    DebugAssert(alignment == Mem::align_up<usize>(alignment, 2), "alignment must be a power of 2");
    DebugAssert(ptr.ptr(), "invalid pointer");

    Header* header = get_header(ptr);
    DebugAssert(header->tags & Allocated, "the given block is already free.");
    
    _check_integrity();
    
    return Mem::align_up(new_size, alignment) <= header->len;
}
        
void GenericAllocator::free(const Slice<u8>& ptr)
{
    DebugAssert(ptr.ptr() != nullptr, "can't delete a null pointer");
    
    Header* header = get_header(ptr);
    DebugAssert(header->tags & Allocated, "the given block is already free.");
    
    header->tags = HeaderTags(0);

    // TODO: Investigate page corruption.
    if(header != nullptr && header->prev != nullptr
        && header->prev->tags == 0)
    {
        header->prev->len += header->len + sizeof(Header);
        header->prev->next = header->next;
        if (header->next != nullptr)
        {
            header->next->prev = header->prev;
        }
        header = header->prev;
    }
    else if(header != nullptr && header->next != nullptr
        && header->next->tags == 0)
    {
        header->len += header->next->len + sizeof(Header);
        header->next = header->next->next;
        if (header->next != nullptr)
        {
            header->next->prev = header;
        }
    }

    _check_integrity();
}

usize GenericAllocator::get_size_of(const Slice<u8>& ptr) const
{
    DebugAssert(ptr.ptr() != nullptr, "can't delete a null pointer");
    
    Header* header = get_header(ptr);
    DebugAssert(header->tags & Allocated, "the given block is freed.");

    return header->len;
}
    
GenericAllocator::Header* GenericAllocator::_search_for_available_space(usize aligned_size, usize alignment)
{
    for(usize i = 0; i < page_count; i++)
    {
        Page& page = allocated_pages[i];
        if(page.first_header == nullptr)
        {
            continue;
        }

        Header* allocated_mem = page.first_header;
        while(allocated_mem != nullptr)
        {
            if (HasValue(allocated_mem->tags & Allocated))
            {
                allocated_mem = allocated_mem->next;
                continue;
            }

            if(allocated_mem->len < aligned_size)
            {
                allocated_mem = allocated_mem->next;
                continue;
            }

            u8* aligned_base = reinterpret_cast<u8*>(
                Mem::align_up(usize(allocated_mem) + sizeof(Header), alignment)
            );
            isize offset = aligned_base - (reinterpret_cast<u8*>(allocated_mem) + sizeof(Header));

            if (offset > 0 && allocated_mem->len >= aligned_size + offset)
            {
                const Header copied_block = *allocated_mem;
                Header* prev = allocated_mem->prev;

                allocated_mem = reinterpret_cast<Header*>(aligned_base - sizeof(Header));
                allocated_mem->len = copied_block.len - offset;
                allocated_mem->page_index = i;
                allocated_mem->tags = 0;
                allocated_mem->index = copied_block.index;

                if(prev != nullptr)
                {
                    prev->len += offset;
                    prev->next = allocated_mem;
                }
                else
                {
                    page.first_header = allocated_mem;
                }

                allocated_mem->prev = copied_block.prev;
                allocated_mem->next = copied_block.next;
                if (allocated_mem->next != nullptr)
                {
                    allocated_mem->next->prev = allocated_mem;
                }

                _check_integrity();
            }
            else if(offset > 0)
            {
                // Try it with the next block
                allocated_mem = allocated_mem->next;
                continue;
            }

            const usize remain = allocated_mem->len - aligned_size;
            
            if (remain >= MinimumValidRemain)
            {
                u8* remain_base = reinterpret_cast<u8*>(aligned_base + aligned_size);
                u8* aligned_remain_base = reinterpret_cast<u8*>(
                    Mem::align_up(usize(remain_base), usize(DefaultAlignmentForRemain))
                );

                const usize offset = aligned_remain_base - remain_base;
                Header* remain_header = reinterpret_cast<Header*>(aligned_remain_base);

                remain_header->len = remain - offset - sizeof(Header);
                remain_header->page_index = allocated_mem->page_index;
                remain_header->tags = 0;

                index++;
                remain_header->index = index;
                remain_header->prev = allocated_mem;
                remain_header->next = allocated_mem->next;

                allocated_mem->len = aligned_size + offset;
                allocated_mem->next = remain_header;

                if (remain_header->next != nullptr)
                {
                    remain_header->next->prev = remain_header;
                }
            }
            
            return allocated_mem;
        }
    }

    return nullptr;
}

GenericAllocator::Page& GenericAllocator::_allocate_new_page(usize size)
{
    Page& page = allocated_pages[page_count++];
    page.bytes = internal_allocator.alloc(size, OS::get_page_size());
    page.first_header = nullptr;
    Log::debug("[Memory]: Page requested at address {} with size {}", page.bytes.ptr(), page.bytes.len);
    return page;
}

void GenericAllocator::_check_integrity()
{
#if DEBUG
    for (usize i = 0; i < page_count; i++)
    {
        usize page_size_accumulator = 0;
        Page& page = allocated_pages[i];
        Header* header = page.first_header;
        while (header != nullptr)
        {
            page_size_accumulator += header->len + sizeof(Header);
            header = header->next;
        }

        if(page_size_accumulator != page.bytes.len)
        {
            Log::debug("Allocation index: {}", index);
            Log::debug("Page({}) with size {} was corrupted, page_size_accumulator was {}", &page, page.bytes.len, page_size_accumulator);
            DebugAssert(page_size_accumulator == page.bytes.len, "the page was corrupted");
        }
    }
#endif
}
    
}