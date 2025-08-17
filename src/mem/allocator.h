#pragma once
#include "core/types.h"

#include <new>


template<typename T>
struct Slice;

namespace mem
{
    struct Allocator
    {
        struct VTable
        {
            Slice<u8>(Allocator::*alloc)(usize, usize);
            bool(Allocator::*realloc)(Slice<u8>, usize, usize);
            void(Allocator::*free)(Slice<u8>);
        };
        
        Slice<u8> alloc(usize size, usize alignment) const;
        bool realloc(Slice<u8> ptr, usize new_size, usize alignment) const;
        void free(Slice<u8> ptr) const;

        template<typename T>
        Slice<T> array(usize count) const;

        template<typename T>
        constexpr void construct_array(Slice<T> array) const;

        template<typename T, typename... TArgs>
        constexpr T* object(TArgs&&... args);
        
        template<typename T, typename... TArgs>
        constexpr void construct(T* instance, TArgs&&... args);

        VTable vtable;
        Allocator* self;
    };
    
}

#include "debug/fail.h"
#include "collections/slice.h"
#include "mem/utils.h"

inline Slice<u8> mem::Allocator::alloc(usize size, usize alignment) const
{
    FailOn(self == nullptr, "self is null");
    Slice<u8> ptr = (self->*vtable.alloc)(size, alignment);
    return ptr;
}

inline bool mem::Allocator::realloc(Slice<u8> ptr, usize new_size, usize alignment) const
{
    FailOn(self == nullptr, "self is null");
    return (self->*vtable.realloc)(ptr, new_size, alignment);
}

inline void mem::Allocator::free(Slice<u8> ptr) const
{
    FailOn(self == nullptr, "self is null");
    
    (self->*vtable.free)(ptr);
}

template<typename T>
Slice<T> mem::Allocator::array(usize count) const
{
    static constexpr usize Alignment = ConditionalValue<usize, alignof(T) == 1, 8, alignof(T)>;
    Slice<T> array = mem::from_bytes<T>(alloc(sizeof(T) * count, Alignment));
    construct_array(array);
    return array;
}

template<typename T>
constexpr void mem::Allocator::construct_array(Slice<T> array) const
{
    ::new(array.ptr()) T[array.len]{};
}

template<typename T, typename... TArgs>
constexpr T* mem::Allocator::object(TArgs&&... args)
{
    T* instance = reinterpret_cast<T*>(alloc(sizeof(T), alignof(T)).items);
    construct(instance, args...);
    return instance;
}

template<typename T, typename... TArgs>
constexpr void mem::Allocator::construct(T* instance, TArgs&&... args)
{
    ::new(instance) T(args...);
}
