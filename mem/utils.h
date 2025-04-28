#pragma once
#include "core/slice.h"

namespace mem
{
    usize get_page_size();
    
    template<typename T>
    inline Slice<u8> to_bytes(const Slice<T>& items)
    {
        return Slice<u8>(
            (u8*)items.items,
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
    constexpr bool compare(Slice<const T> src1, Slice<const T> src2)
    {
        if(src1.len != src2.len)
        {
            return false;
        }
        
        for(usize i = 0; i < src1.len; i++)
        {
            if(src1[i] != src2[i])
            {
                return false;
            }
        }
        
        return true;
    }


    template<typename T>
    constexpr void copy(Slice<T> dest, const Slice<T>& src)
    {
        DebugAssert(dest.len >= src.len, "Invalid destination");

        for(usize i = 0; i < src.len; i++)
        {
            dest[i] = src[i];
        }
    }

    template<typename T>
    constexpr void set(Slice<T> dest, const T value)
    {
        for(usize i = 0; i < dest.len; i++)
        {
            dest[i] = value;
        }
    }

}
