#pragma once
#include "core/types.h"
#include "core/templates.h"


template<typename T>
struct Slice;

namespace mem
{

struct Allocator
{
    static constexpr usize DefaultAlignment = sizeof(MemoryAddress) * 2;

    struct VTable
    {
        Slice<u8>(Allocator::*alloc)(usize, usize);
        bool(Allocator::*realloc)(Slice<u8>, usize, usize);
        void(Allocator::*free)(Slice<u8>);
        usize(Allocator::*get_size_of)(Slice<u8> ptr);
    };
        
    /*
    * Allocator API
    */
    Slice<u8> alloc(usize size, usize alignment) const;
    bool realloc(Slice<u8> ptr, usize new_size, usize alignment) const;
    void free(Slice<u8> ptr) const;
    usize get_size_of(Slice<u8> ptr) const;

    template<typename T>
    Slice<T> array(usize count) const;

    template<typename T, typename... TArgs>
    constexpr T* object(TArgs&&... args) const;
        
    VTable* vtable;
    Allocator* self;
};

}

#include "debug/assertion.h"
#include "collections/slice.h"
#include "mem/utils.h"

inline Slice<u8> mem::Allocator::alloc(usize size, usize alignment) const
{
    DebugAssert(self != nullptr, "self is null");
    Slice<u8> ptr = (self->*vtable->alloc)(size, alignment);
    return ptr;
}

inline bool mem::Allocator::realloc(Slice<u8> ptr, usize new_size, usize alignment) const
{
    DebugAssert(self != nullptr, "self is null");
    return (self->*vtable->realloc)(ptr, new_size, alignment);
}

inline void mem::Allocator::free(Slice<u8> ptr) const
{
    DebugAssert(self != nullptr, "self is null");
    (self->*vtable->free)(ptr);
}

inline usize mem::Allocator::get_size_of(Slice<u8> ptr) const
{
    DebugAssert(self != nullptr, "self is null");
    return (self->*vtable->get_size_of)(ptr);
}

template<typename T>
inline Slice<T> mem::Allocator::array(usize count) const
{
    static constexpr usize Alignment = ConditionalValue<usize, alignof(T) == 1, 8, alignof(T)>;
    Slice<T> array = mem::from_bytes<T>(alloc(sizeof(T) * count, Alignment));
    ConstructArray(array.ptr(), array.len);
    return array;
}

template<typename T, typename... TArgs>
inline constexpr T* mem::Allocator::object(TArgs&&... args) const
{
    constexpr usize alignment = alignof(T) == 1 ? 16 : alignof(T);
    T* instance = reinterpret_cast<T*>(alloc(sizeof(T), alignment).items);
    ConstructObject(*instance, args...);
    return instance;
}

