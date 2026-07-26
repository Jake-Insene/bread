#pragma once
#include "core/header.h"


template<typename T>
struct Slice;

namespace Mem
{

struct Allocator
{
    static constexpr usize DefaultAlignment = sizeof(MemoryAddress) * 2;
    
    /*
    * Allocator API
    */

    /*
    * Allocates a slice of bytes of len size, the base address is aligned to alignment.
    * The return slice is not garanted to be all zero.
    */
    virtual Slice<u8> alloc(usize size, usize alignment) = 0;
    /*
    * Try to expand the memory block, if it is expanded, the new memory is not garanted to be all zero.
    * It will return whenever the memory block is expanded, otherwise it will return false.
    */
    virtual bool realloc(const Slice<u8>& ptr, usize new_size, usize alignment) = 0;

    /*
    * Try to expand the memory block, if it is expanded, the new memory is not garanted to be all zero.
    * otherwise it will allocate a new memory block and copy the old data to the new one, and free the old memory block.
    * The return slice is not garanted to be all zero.
    */
    virtual Slice<u8> remap(const Slice<u8>& ptr, usize new_size, usize alignment) = 0;

    /*
    * Frees the memory block, the ptr must be allocated by this allocator.
    */
    virtual void free(const Slice<u8>& ptr) = 0;

    template<typename T>
    Slice<T> array(usize count);

    template<typename T, typename... TArgs>
    T* object(TArgs&&... args);
};

}

#include "debug/assertion.h"
#include "collections/slice.h"
#include "mem/utils.h"

template<typename T>
inline Slice<T> Mem::Allocator::array(usize count)
{
    static constexpr usize Alignment = ConditionalValue<usize, alignof(T) == 1, 8, alignof(T)>;
    Slice array = Mem::from_bytes<T>(alloc(sizeof(T) * count, Alignment));
    ConstructArray(array.ptr(), array.len);
    return array;
}

template<typename T, typename... TArgs>
inline T* Mem::Allocator::object(TArgs&&... args)
{
    constexpr usize alignment = alignof(T) == 1 ? 16 : alignof(T);
    T* instance = reinterpret_cast<T*>(alloc(sizeof(T), alignment).items);
    ConstructObject(*instance, args...);
    return instance;
}

