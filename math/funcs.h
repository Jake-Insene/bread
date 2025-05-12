#pragma once
#include "core/templates.h"
#include "core/types.h"
#include "core/macros.h"

#include <cmath>

namespace math
{
    template<typename T>
    constexpr T PI = T(3.1415926535);
    
    template<typename T>
    constexpr T abs(T a)
    {
        static_assert(IsArithmetic<T>, "expected arithmetic type");
        return a >= 0 ? a : -a;
    }

    template<typename T>
    [[nodiscard]] constexpr T log2(T n)
    {
        static_assert(
            IsArithmetic<T>,
            "expected arithmetic type"
            );

        if constexpr (IsInteger<T>)
        {
            T result = 0;
            while (n >>= 1)
            {
                result++;
            }
            return result;
        }
        else
        {
            // type is floating point;
        }
    }
    
    template<typename T>
    [[nodiscard]] constexpr T next_pow2(const T n)
    {
        static_assert(
            IsArithmetic<T>,
            "expected arithmetic type"
            );

        if (n == 0)
        {
            return 1;
        }

        T p = 1;
        while (p < n)
        {
            p *= 2;
        }

        return p;
    }
    
    template<typename T>
    [[nodiscard]] constexpr T sqrt(T v)
    {
        return (T)::sqrt(v);
    }
    
    template<typename T>
    [[nodiscard]] constexpr T rads(const T degrees)
    {
        static_assert(
            IsArithmetic<T>,
            "expected arithmetic type"
        );
        return (PI<T> * degrees) / 180;
    }

    template<typename T>
    [[nodiscard]] constexpr T degrees(const T rads)
    {
        static_assert(
            IsArithmetic<T>,
            "expected arithmetic type"
            );
        return (rads * 180) / PI<T>;

    }
    
    template<typename T>
    [[nodiscard]] constexpr T cos(T r)
    {
        static_assert(
            IsArithmetic<T>,
            "expected arithmetic type"
        );
        return (T)::cos(f32(r));
    }
    
    template<typename T>
    [[nodiscard]] constexpr T sin(T r)
    {
        static_assert(
            IsArithmetic<T>,
            "expected arithmetic type"
        );
        return (T)::sin(f32(r));
    }
    
    template<typename T>
    [[nodiscard]] constexpr T atan2(T x, T y)
    {
        return (T)::atan2(x, y);
    }
    
    template<typename T>
    [[nodiscard]] constexpr T max(T a, T b)
    {
        return a > b ? a : b;
    }

    template<typename T>
    [[nodiscard]] constexpr T min(T a, T b)
    {
        return a < b ? a : b;
    }
    
    template<typename T>
    [[nodiscard]] constexpr T sign(T v)
    {
        return v >= 0 ? 1 : -1;
    }
}
