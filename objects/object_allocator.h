#pragma once
#include "mem/page_allocator.h"
#include "mem/generic_allocator.h"
#include "objects/object.h"
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
    
    static inline ObjectChunk chunks[MaxObjectChunks];
    
    static inline mem::PageAllocator internal_object_allocator;
    static inline mem::GenericAllocator object_allocator;
    
    static void initialize();
    static void shutdown();
    
    static Object* allocate_class(const Object::Class* klass);
    
    static Object* allocate_object(const Object::Class* klass);
    
    static void destroy_object(Object* obj);
    
    static void allocate_new_block(
        ObjectBlock& block, usize count, usize object_size
    );
    
    static Object* get_by_id(ObjectID& id);
};

// Create an object of the given type.
// It don't put it in the scene tree, you must explicitly call add_child.
template<typename T>
[[nodiscard]] inline T* CreateObject()
{
    static_assert(!IsSame<Object, T>, "You cannot allocate an Object class directly");
    return reinterpret_cast<T*>(
        ObjectAllocator::allocate_class(
            T::get_class()
        )
    );
}

// Destroy the given object.
// You should use this only if obj is not in the scene,
// use remove_child instead.
inline void DestroyObject(Object* obj)
{
    ObjectAllocator::destroy_object(obj);
}


// Get the object referenced by the id.
template<typename T>
[[nodiscard]] inline T* GetObjectByID(ObjectID id)
{
    return reinterpret_cast<T*>(ObjectAllocator::get_by_id(id));
}

