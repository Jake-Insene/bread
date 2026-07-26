#include "mem/generic_allocator.h"

#include "debug/fail.h"
#include "debug/log.h"
#include "mem/utils.h"
#include "os/os.h"


// TODO: Free memory is filled with 0xDD
// Allocated but not initialized with 0xCD

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
        if(page.first_header != nullptr)
        {
            _check_integrity(page.first_header);
        }

        Header* header = page.first_header;
        usize header_count = 0;
        while(header != nullptr)
        {
            header_count++;
            header = header->next;
        }

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

    Header* allocated_mem_header = _search_for_available_space(aligned_size, alignment);
    if(allocated_mem_header != nullptr)
    {
        // allocated_mem = aligned_base - sizeof(Header)
        u8* base = reinterpret_cast<u8*>(usize(allocated_mem_header) + sizeof(Header));

        allocated_mem_header->tags |= Allocated;
        _check_integrity(allocated_mem_header);

        index++;
        allocated_mem_header->index = index;

#if defined(DEBUG)
        Mem::set(Slice(base, size), u8(0xCD));
#endif
        return Slice
        {
            base,
            size
        };
    }

    Page& new_page = _allocate_new_page(next_page_size + aligned_size + sizeof(Header));
    next_page_size *= 2;

    u8* base = new_page.bytes.ptr();
    Header* allocation_header = reinterpret_cast<Header*>(base);
    // header is always aligned to a power of two, so we can just add sizeof(Header)
    // to the base address to get the aligned memory address.
    u8* aligned_mem = reinterpret_cast<u8*>(base) + sizeof(Header);

    allocation_header->len = new_page.bytes.len - sizeof(Header);
    allocation_header->page_index = page_count - 1;
    allocation_header->tags = Allocated;
    allocation_header->prev = nullptr;
    allocation_header->next = nullptr;
    
    new_page.first_header = allocation_header;

    // Trying to dividing the memory if the requested memory is too low
    usize unused_size = allocation_header->len - aligned_size;
    if(unused_size >= MinimumValidRemain)
    {
        // Creating new header
        Header* fill_header = reinterpret_cast<Header*>(aligned_mem + aligned_size);
        fill_header->len = unused_size - sizeof(Header);
        fill_header->page_index = allocation_header->page_index;
        fill_header->tags = 0;
        fill_header->index = index++;
        fill_header->prev = allocation_header;
        fill_header->next = nullptr;

        allocation_header->len = aligned_size;
        allocation_header->next = fill_header;
    }
    
    _check_integrity(allocation_header);

    index++;
    allocation_header->index = index;

#if defined(DEBUG)
    Mem::set(Slice(aligned_mem, size), u8(0xCD));
#endif
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
    
    return Mem::align_up(new_size, alignment) <= header->len;
}

Slice<u8> GenericAllocator::remap(const Slice<u8>& ptr, usize new_size, usize alignment)
{
    DebugAssert(alignment == Mem::align_up<usize>(alignment, 2), "alignment must be a power of 2");
    DebugAssert(ptr.ptr(), "invalid pointer");

    Header* header = get_header(ptr);
    DebugAssert(header->tags & Allocated, "the given block is already free.");

    if(Mem::align_up(new_size, alignment) <= header->len)
    {
        return ptr;
    }

    Slice<u8> new_mem = alloc(new_size, alignment);
    Mem::copy(new_mem, Slice(ptr.ptr(), header->len));
    free(ptr);
    return new_mem;
}
        
void GenericAllocator::free(const Slice<u8>& ptr)
{
    DebugAssert(ptr.ptr() != nullptr, "can't delete a null pointer");
    
    Header* header = get_header(ptr);
    DebugAssert(header->tags & Allocated, "the given block is already free.");
    
    header->tags = HeaderTags();

    // TODO: Investigate page corruption.
    while(header != nullptr && header->prev != nullptr
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

    while(header != nullptr && header->next != nullptr
        && header->next->tags == 0)
    {
        header->len += header->next->len + sizeof(Header);
        header->next = header->next->next;

        if(header->next != nullptr)
        {
            header->next->prev = header;
        }
    }

    _check_integrity(header);
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

        if(Header* header = _search_for_space_in_page(page, aligned_size, alignment))
        {
            return header;
        }
    }

    return nullptr;
}

GenericAllocator::Header* GenericAllocator::_search_for_space_in_page(Page& page, usize aligned_size, usize alignment)
{
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

        u8* base = reinterpret_cast<u8*>(allocated_mem) + sizeof(Header);
        u8* aligned_mem = reinterpret_cast<u8*>(Mem::align_up<usize>(usize(base), alignment));
        isize offset = aligned_mem - base;

        if(offset < 0)
        {
            allocated_mem = allocated_mem->next;
            continue;
        }

        if(offset > 0 && allocated_mem->len < aligned_size + offset)
        {
            allocated_mem = allocated_mem->next;
            continue;
        }

        // We can align at the beginning
        if(offset > 0 && page.first_header == allocated_mem)
        {
            allocated_mem = allocated_mem->next;
            continue;
        }

        if(offset == 0)
        {
            allocated_mem->index = index++;
            allocated_mem->tags |= Allocated;
            _check_integrity(allocated_mem);

            isize remain = allocated_mem->len - aligned_size;
            if(remain > 0)
            {
                u8* remain_base = reinterpret_cast<u8*>(Mem::align_up<usize>(usize(aligned_mem + aligned_size), alignof(Header)));
                usize padding = usize(remain_base - (aligned_mem + aligned_size));
                isize remain_total = allocated_mem->len - aligned_size - padding;

                if(remain_total > 0 && usize(remain_total) >= MinimumValidRemain)
                {
                    Header* remain_header = reinterpret_cast<Header*>(remain_base);
                    remain_header->len = remain_total - sizeof(Header);
                    remain_header->page_index = allocated_mem->page_index;
                    remain_header->tags = 0;
                    remain_header->index = index++;
                    remain_header->prev = allocated_mem;
                    remain_header->next = allocated_mem->next;

                    if(allocated_mem->next != nullptr)
                    {
                        allocated_mem->next->prev = remain_header;
                    }

                    allocated_mem->len = aligned_size + padding;
                    allocated_mem->next = remain_header;
                }
            }

            return allocated_mem;
        }

        // Normal align code
        // prev_header is always non null because we check for the first_header above
        Header* prev_header = allocated_mem->prev;
        Header* next_header = allocated_mem->next;
        Header old_header = *allocated_mem;

        //if(prev_header->tags & Allocated)
        //{
        //    allocated_mem = allocated_mem->next;
        //    continue;
        //}

        Header* new_header = reinterpret_cast<Header*>(aligned_mem - sizeof(Header));
        usize expand_size = usize(new_header) - usize(allocated_mem);

        new_header->len = old_header.len - offset;
        new_header->page_index = old_header.page_index;
        new_header->tags = old_header.tags;
        new_header->index = index++;
        new_header->prev = prev_header;
        new_header->next = next_header;

        prev_header->len += expand_size;
        prev_header->next = new_header;
        if(next_header != nullptr)
        {
            next_header->prev = new_header;
        }

        isize remain = new_header->len - aligned_size;
        if(remain > 0)
        {
            u8* remain_base = reinterpret_cast<u8*>(Mem::align_up<usize>(usize(aligned_mem + aligned_size), alignof(Header)));
            usize padding = usize(remain_base - (aligned_mem + aligned_size));
            isize remain_total = new_header->len - aligned_size - padding;

            if(remain_total > 0 && usize(remain_total) >= MinimumValidRemain)
            {
                Header* remain_header = reinterpret_cast<Header*>(remain_base);
                remain_header->len = remain_total - sizeof(Header);
                remain_header->page_index = new_header->page_index;
                remain_header->tags = 0;
                remain_header->index = index++;
                remain_header->prev = new_header;
                remain_header->next = new_header->next;

                if(new_header->next != nullptr)
                {
                    new_header->next->prev = remain_header;
                }

                new_header->len = aligned_size + padding;
                new_header->next = remain_header;
            }
        }

        return new_header;
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

void GenericAllocator::_check_integrity(Header* header)
{
#if DEBUG
    Page& page = allocated_pages[header->page_index];

    usize page_size_accumulator = 0;
    Header* page_header = page.first_header;
    while (page_header != nullptr)
    {
        page_size_accumulator += page_header->len + sizeof(Header);
        page_header = page_header->next;
    }

    if(page_size_accumulator != page.bytes.len)
    {
        Log::debug("Allocation index: {}", index);
        Log::debug("Page({}) with size {} was corrupted, page_size_accumulator was {}", page.bytes.ptr(), page.bytes.len, page_size_accumulator);
        DebugAssert(page_size_accumulator == page.bytes.len, "the page was corrupted");
    }
#endif
}
    
}