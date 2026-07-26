#pragma once
#include "mem/page_allocator.h"


namespace Mem
{
    
struct GenericAllocator : Mem::Allocator
{
    static constexpr u16 MaxPageCount = MaxValue<u16>;
    static constexpr usize DefaultPageListSize = 128ULL;
    static constexpr usize DirectPageAllocationSize = 32ULL * 1024ULL; // 256 KB
    static constexpr usize DefaultNextPageSize = 1024ULL * 16ULL; // 16 KB
    static constexpr usize DefaultAlignmentForRemain = 16ULL;

    enum HeaderTags
    {
        Allocated = Bit(0),
    };
    
    struct Header
    {
        usize len;
        usize page_index : 16;
        usize tags : 16;
        usize index : 32;
        Header* prev;
        Header* next;
    };
    static_assert(sizeof(Header) == 32, "invalid header alignment");
    
    static constexpr usize MinimumValidRemain = Mem::align_up(sizeof(Header) * 2, alignof(Header));
    
    struct Page
    {
        Slice<u8> bytes;
        Header* first_header;
    };

    PageAllocator internal_allocator;
    Slice<Page> allocated_pages;
    usize page_count;
    usize next_page_size;
    u32 index;

    void init();
    void destroy();

    virtual Slice<u8> alloc(usize size, usize alignment) override;
    virtual bool realloc(const Slice<u8>& ptr, usize new_size, usize alignment) override;
    virtual Slice<u8> remap(const Slice<u8>& ptr, usize new_size, usize alignment) override;
    virtual void free(const Slice<u8>& ptr) override;

    Header* _search_for_available_space(usize aligned_size, usize alignment);
    Header* _search_for_space_in_page(Page& page, usize aligned_size, usize alignment);

    Page& _allocate_new_page(usize size);
    void _check_integrity(Header* header);
};
    
}

EnableBitOp(Mem::GenericAllocator::HeaderTags)
