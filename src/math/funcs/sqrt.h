#pragma once
#include "math/funcs/abs.h"


namespace math
{

template<typename T>
[[nodiscard]] constexpr T sqrt(T n)
{
    static_assert(IsFloatingPoint<T>, "expected floating point type");
    // TODO: Improve this
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
}

}