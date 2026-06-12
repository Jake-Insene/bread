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
    virtual Slice<u8> alloc(usize size, usize alignment) = 0;
    virtual bool realloc(const Slice<u8>& ptr, usize new_size, usize alignment) = 0;
    virtual void free(const Slice<u8>& ptr) = 0;
    virtual usize get_size_of(const Slice<u8>& ptr) const = 0;

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

