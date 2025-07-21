#pragma once
#include "core/types.h"

template<typename T>
struct [[nodiscard]] ColorT
{
    T r;
    T g;
    T b;
    T a;
};

using Color = ColorT<u8>;
using ColorF = ColorT<f32>;
