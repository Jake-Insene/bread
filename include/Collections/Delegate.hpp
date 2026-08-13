#pragma once
#include "Collections/Tuple.hpp"
#include "Mem/Allocator.hpp"


template<typename Fn>
struct Delegate;

template<typename T, typename... TArgs>
struct Delegate<T(TArgs...)>
{
    Mem::Allocator& allocator;

    T(*func)(Core::Opaque*, TArgs&&...);
    Slice<u8> reserved;

    static Delegate create(Mem::Allocator& allocator)
    {
        return Delegate
        {
            .allocator = allocator,
            .func = nullptr,
            .reserved = {},
        };
    }

    Delegate(Mem::Allocator& allocator)
    : allocator(allocator), func(nullptr), reserved()
    {}

    ~Delegate()
    {
        if(reserved.ptr())
        {
            allocator.free(reserved);
        }
    }

    template<typename Fn>
    void bind(Fn&& fn)
    {
        _try_reserve(sizeof(Fn));
        Core::Mem::Placement(*reinterpret_cast<Fn*>(reserved.ptr()), fn);

        func = [](Core::Opaque* opaque, TArgs&&... args)
        {
            Fn* func = opaque->cast<Fn*>();
            (*func)(args...);
        };
    }

    T call(TArgs&&... args)
    {
        return func(reinterpret_cast<Core::Opaque*>(reserved.ptr()), Core::Forward<TArgs>(args)...);
    }

    void _try_reserve(usize size)
    {
        if(reserved.len >= size)
        {
            return;
        }

        if(reserved.len < size && !reserved.null())
        {
            allocator.free(reserved);
        }

        reserved = allocator.array<u8>(size);
    }
};
