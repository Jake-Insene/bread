#pragma once
#include "mem/page_allocator.h"
#include "mem/generic_allocator.h"
#include "object/object.h"
#include "math/funcs.h"


struct ObjectAllocator
{
    static constexpr usize MaxObjectChunks = math::log2(sizeof(Object));
    static constexpr usize ChunkBase = math::log2(math::next_pow2(sizeof(Object)));
    static constexpr usize MaxObjectBlocks = 12;
    static constexpr usize ObjectBlockBaseCount = 256;
    
    struct ObjectBlock
    {
        Slice<u8> bytes;
        usize count;
        usize index;
    };
    
    struct ObjectChunk
    {
        ObjectBlock blocks[MaxObjectBlocks];
        ObjectID last_free_id;
        usize object_size;
    };

    struct InternalData
    {
        ObjectChunk chunks[MaxObjectChunks];
        mem::PageAllocator internal_object_allocator;
        mem::GenericAllocator object_allocator;
    };
    
    static inline InternalData data;
    
    static void initialize();
    static void shutdown();
    
    static Object* allocate_class(const Object::Class* klass);
    
    static Object* allocate_object(const Object::Class* klass);
    
    static void destroy_object(Object* obj);
    
    static void allocate_new_block(
        ObjectBlock& block, usize count, usize object_size
    );
    
    static Object* get_by_id(ObjectID id);

    static Object* _get_by_id_no_alloc(ObjectID id);
    static Object* _request_new_object(const Object::Class* klass);
};


