#pragma once
#include "Collections/Slice.hpp"
#include "Core/Templates.hpp"


template<typename T>
struct Slice;

namespace Mem
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

/**
* @brief Performs a element by element comparison, returns false at the first mismatch. 
*/
template<typename T>
constexpr bool compare(Slice<const T> src1, Slice<const T> src2);

/**
* @brief Performs a byte by byte copy from src to dest.
* @param dest The destination range of elments.
* @param src The source range of elments.
*/
template<typename T, typename U>
constexpr void copy(Slice<T> dest, const Slice<U>& src);

/**
* @brief Performs a element by element set.
* @param dest The destination range of elements.
* @param value Source T value.
*/
template<typename T>
constexpr void set(Slice<T> dest, T value);

/**
* @brief Performs a byte by byte zero fill.
*/
template<typename T>
constexpr void zero(Slice<T> dest);

template<typename T>
constexpr void swap(T& a, T& b)
{
    T tmp = a;
    a = b;
    b = tmp;
}

}

namespace Mem
{

template<typename T>
inline Slice<u8> to_bytes(const Slice<T>& items)
{
    return Slice
    {
        reinterpret_cast<u8*>(items.items),
        items.len * sizeof(T)
    };
}

template<typename T>
inline Slice<const u8> to_const_bytes(const Slice<T>& items)
{
    return Slice
    {
        reinterpret_cast<const u8*>(items.items),
        items.len * sizeof(T)
    };
}

template<typename T>
inline Slice<T> from_bytes(const Slice<u8>& bytes)
{
    return Slice
    {
        reinterpret_cast<T*>(bytes.items),
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
constexpr void _copy_impl(Slice<T> dest, const Slice<const T>& src)
{
    DebugAssert(dest.len >= src.len, "invalid destination");
    Slice dest_bytes = to_bytes(dest);
    Slice src_bytes = to_const_bytes(src);

#if BREAD_ENABLE_INTRISICS
    if !consteval
    {
        _copy(dest_bytes, src_bytes);
        return;
    }
    else
#endif
    {
        for (usize i = 0; i < src_bytes.len; i++)
        {
            dest_bytes[i] = src_bytes[i];
        }
    }
}

template<typename T, typename U>
constexpr void copy(Slice<T> dest, const Slice<U>& src)
{
    _copy_impl<T>(dest, Slice<const T>(src.items, src.len));
}

void _set(Slice<u8> dest, u8 value);

template<typename T>
constexpr void set(Slice<T> dest, const T value)
{
#if BREAD_ENABLE_INTRISICS
    if !consteval
    {
        if constexpr(Core::IsSame<Core::RemoveCVRef<T>, u8>) // for floating point values it works
        {
            Slice dest_bytes = to_bytes(dest);
            _set(dest_bytes, value);
            return;
        }
    }
#endif

    for (usize i = 0; i < dest.len; i++)
    {
        dest[i] = value;
    }
}

template<typename T>
constexpr void zero(Slice<T> dest)
{
    Slice dest_bytes = to_bytes(dest);
    
#if BREAD_ENABLE_INTRISICS
    if !consteval
    {
        _set(dest_bytes, 0);
        return;
    }
#endif

    for (usize i = 0; i < dest_bytes.len; i++)
    {
        dest_bytes[i] = 0;
    }
}

}
