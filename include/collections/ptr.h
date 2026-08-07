#pragma once
#include "collections/tuple.h"
#include "mem/allocator.h"
#include "mem/utils.h"


/*
* Abstracts a pointer type, should be used only with Scoped<T>.
* The structure OWNS the memory pointer and will manages its life time.
* Ptr doesn't save the allocator that owns the memory you must pass the
* allocator that owns it explicitly in any operation of the Ptr class.
*/
template<typename T>
struct Ptr
{
    T* memory;
#if defined(DEBUG)
    Mem::Allocator& allocator;
#endif

    template<typename... TArgs>
    static Ptr<T> create(Mem::Allocator& allocator, TArgs&&... args)
    {
        T* memory = allocator.object<T>(Forward<TArgs>(args)...);
        
        return Ptr<T>
        {
            .memory = memory,
#if defined(DEBUG)
            .allocator_self = allocator,
#endif
        };
    }

    template<typename... TArgs>
    static Ptr<T> from_memory(Mem::Allocator& allocator, T* memory)
    {
#if defined(RELEASE)
        Unused(allocator);
#endif
        return Ptr<T>
        {
            .memory = memory,
#if defined(DEBUG)
            .allocator = allocator,
#endif
        };
    }

    static Ptr<T> from_raw(T* memory)
    {
        return Ptr<T>
        {
            .memory = memory,
#if defined(DEBUG)
            // This ptr should never be deallocate.
            .allocator_self = nullptr,
#endif
        };
    }

    void destroy(Mem::Allocator& allocator)
    {
        DebugAssert(allocator == allocator, "allocator mismatch");
        DebugAssert(memory != nullptr, "memory is null");

        DestructObject(*memory);

        allocator.free(Mem::to_bytes(Slice(memory, 1)));
        memory = nullptr;
    }

    T* get() const
    { 
        DebugAssert(memory != nullptr, "memory is null"); 
        return memory; 
    }
};
