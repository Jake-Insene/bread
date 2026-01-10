#pragma once
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
		Unused(Forward<TArgs>(args)...);
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

    template<typename... TArgs>
    static Ptr<T> create(mem::Allocator& allocator, TArgs&&... args)
    {
        T* memory = allocator.object<T>(Forward<TArgs>(args)...);

        return Ptr<T>
        {
            .memory = memory,
        };
    }

    void destroy(const mem::Allocator& allocator)
    {
        DebugAssert(memory != nullptr, "memory is null");

        allocator.free(mem::to_bytes(Slice<T>(memory, 1)));
        memory = nullptr;
    }

    inline T* operator->() const { return memory; }
};
