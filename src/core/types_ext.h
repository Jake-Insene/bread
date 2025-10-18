#pragma once
#include "core/types.h"


struct Opaque
{
    void* self;

    constexpr Opaque() : self(nullptr) {}
    constexpr Opaque(void* data) : self(data) {}

    [[nodiscard]] constexpr operator void* () { return self; }

    template<typename T>
    [[nodiscard]] T cast() { return reinterpret_cast<T>(self); };
};