#pragma once
#include "math/funcs/abs.h"
#include "platform/instrinsics.h"


namespace Math
{

//TODO: Make the constexpr version perfomance
template<typename T>
requires(Core::IsFloatingPoint<T>)
[[nodiscard]] constexpr T sqrt(T n)
{
#if BREAD_ENABLE_INTRISICS
    if !consteval
    {
        return PlatformIntrisics::sqrt(n);
    }
#endif

    if (n == T(0))
    {
        return n;
    }

    const T tolerance = T(1e-10);
    T x = n;

    while (true)
    {
        const T root = T(0.5) * (x + (n / x));

        if (Math::abs(root - x) < tolerance)
        {
            return root;
        }

        x = root;
    }
}

}