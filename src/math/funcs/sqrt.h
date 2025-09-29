#pragma once
#include "math/funcs/abs.h"
#include "platform/instrinsics.h"



namespace math
{


template<typename T>
    requires(IsFloatingPoint<T>)
[[nodiscard]] constexpr T sqrt(T x)
{
    // TODO: Improve this

#if BREAD_ENABLE_INTRISICS
    return PlatformIntricics::sqrt(x);
#else
    if (n == T(0))
        return n;

    const T tolerance = T(1e-10);
    T x = n;

    while (true)
    {
        const T root = T(0.5) * (x + (n / x));

        if (math::abs(root - x) < tolerance)
            return root;

        x = root;
    }
#endif
}

}