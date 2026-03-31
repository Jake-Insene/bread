#pragma once
#include "collections/tuple.h"
#include "mem/allocator.h"
#include "mem/utils.h"


template<typename T>
struct Ptr;

template<typename T>
struct ScopedData;

template<typename T>
struct ScopedData<Ptr<T>>
{
	using DestroyArgList = TypeList<mem::Allocator>;
	
	mem::Allocator allocator;

	ScopedData() : allocator() {}

	template<typename... TArgs>
	ScopedData(const mem::Allocator& allocator, TArgs&&... args) : allocator(allocator)
	{
		Unused(args...);
	}

	auto as_tuple() const { return Tuple<mem::Allocator>(allocator); }
};


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
    void* allocator_self;
#endif

    template<typename... TArgs>
    static Ptr<T> create(mem::Allocator& allocator, TArgs&&... args)
    {
        T* memory = allocator.object<T>(Forward<TArgs>(args)...);
        
        return Ptr<T>
        {
            .memory = memory,
#if defined(DEBUG)
            .allocator_self = allocator.self,
#endif
        };
    }

    template<typename... TArgs>
    static Ptr<T> from_memory(mem::Allocator& allocator, T* memory)
    {
#if defined(RELEASE)
        Unused(allocator);
#endif
        return Ptr<T>
        {
            .memory = memory,
#if defined(DEBUG)
            .allocator_self = allocator.self,
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

    void destroy(const mem::Allocator& allocator)
    {
        DebugAssert(allocator.self == allocator_self, "allocator mismatch");
        DebugAssert(memory != nullptr, "memory is null");

        DestructObject(*memory);

        allocator.free(mem::to_bytes(Slice<T>(memory, 1)));
        memory = nullptr;
    }

    inline T* get() const
    { 
        DebugAssert(memory != nullptr, "memory is null"); 
        return memory; 
    }
};
