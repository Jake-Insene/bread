#pragma once
#include "core/types.h"


template<typename T>
struct Slice;

namespace mem
{
    template<typename T>
    inline Slice<u8> to_bytes(const Slice<T>& items);

    template<typename T>
    inline Slice<const u8> to_const_bytes(const Slice<T>& items);
    
    template<typename T>
    inline Slice<T> from_bytes(const Slice<u8>& bytes);
    
    template<typename T>
    constexpr T align_up(T value, T alignment)
    {
        return (value + (alignment - 1)) & ~(alignment - 1);
    }
    
    template<typename T>
    constexpr T align_down(T value, T alignment)
    {
        return value & ~(alignment - 1);
    }
    
    template<typename T>
    constexpr bool compare(Slice<const T> src1, Slice<const T> src2);

    template<typename T>
    inline void copy(Slice<T> dest, const Slice<const T>& src);

    template<typename T>
    inline void copy(Slice<T> dest, const Slice<T>& src);

    template<typename T>
    inline void set(Slice<T> dest, const T value);

}


#include "collections/slice.h"
#include "debug/debug.h"

namespace mem
{

template<typename T>
inline Slice<u8> to_bytes(const Slice<T>& items)
{
    return Slice<u8>(
        (u8*)items.items,
        items.len * sizeof(T)
    );
}

template<typename T>
inline Slice<const u8> to_const_bytes(const Slice<T>& items)
{
    return Slice<const u8>(
        (const u8*)items.items,
        items.len * sizeof(T)
    );
}

template<typename T>
inline Slice<T> from_bytes(const Slice<u8>& bytes)
{
    return Slice<T>
    {
        (T*)bytes.items,
            bytes.len / sizeof(T),
    };
}

template<typename T>
constexpr bool compare(Slice<const T> src1, Slice<const T> src2)
{
    if (src1.len != src2.len)
    {
        return false;
    }

    for (usize i = 0; i < src1.len; i++)
    {
        if (src1[i] != src2[i])
        {
            return false;
        }
    }

    return true;
}

void _copy(Slice<u8> dest, Slice<const u8> src);

template<typename T>
inline void copy(Slice<T> dest, const Slice<const T>& src)
{
    DebugAssert(dest.len >= src.len, "invalid destination");
#if BREAD_ENABLE_INTRISICS
    _copy(to_bytes(dest), to_const_bytes(src));
#else
    for (usize i = 0; i < src.len; i++)
    {
        dest[i] = src[i];
    }
#endif
}

template<typename T>
inline void copy(Slice<T> dest, const Slice<T>& src)
{
    DebugAssert(dest.len >= src.len, "invalid destination");
#if BREAD_ENABLE_INTRISICS
    _copy(to_bytes(dest), to_const_bytes(src));
#else
    for (usize i = 0; i < src.len; i++)
    {
        dest[i] = src[i];
    }
#endif
}

void _set_zero(Slice<u8> dest);

template<typename T>
inline void set(Slice<T> dest, const T value)
{
#if BREAD_ENABLE_INTRISICS
    if (value == T(0)) // for floating point values it works
    {
        _set_zero(to_bytes(dest));
        return;
    }
#endif

    for (usize i = 0; i < dest.len; i++)
    {
        dest[i] = value;
    }
}

}
