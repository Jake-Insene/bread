#pragma once
#include "Core/Header.hpp"


template<typename T>
requires(Core::IsArithmetic<T>)
struct [[nodiscard]] ColorT
{
    T r;
    T g;
    T b;
    T a;

    constexpr ColorT()
        : r(0), g(0), b(0), a(0)
    {}

    explicit constexpr ColorT(T r, T g, T b, T a)
        : r(r), g(g), b(b), a(a)
    {}
};

using Color = ColorT<u8>;
using ColorF = ColorT<f32>;
