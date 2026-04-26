#pragma once
#include "math/mat4.h"
#include "math/vec3.h"

// Projection like Mat4 is in row major order
struct [[nodiscard]] Projection : Mat4
{
    static constexpr Projection orthographic(
        const f32 left, const f32 right, const f32 bottom, const f32 top,
        const f32 near, const f32 far
    )
    {
        Projection matrix = Projection
        {
            Vector4(2.F / (right - left), 0, 0, -(right + left) / (right - left)),
            Vector4(0, 2.F / (top - bottom), 0, -(top + bottom) / (top - bottom)),
            Vector4(0, 0, -1 / (far - near),  -near) / (far - near),
            Vector4(0, 0, 0, 1)
        };

        return matrix;
    }

    static constexpr Projection orthographic_inv(
        const f32 left, const f32 right, const f32 bottom, const f32 top,
        const f32 near, const f32 far
    )
    {
        Projection matrix = Projection
        {
            Vector4((right - left) / 2.F, 0, 0, (left + right) / 2.F),
            Vector4(0, (top - bottom) / 2.F, 0, (top + bottom) / 2.F),
            Vector4(0, 0, (far - near) / -1,  -(far + near) / 1.F),
            Vector4(0, 0, 0, 1)
        };

        return matrix;
    }
    
    constexpr Projection() : Mat4() {}
    constexpr Projection(const Vector4& _00, const Vector4& _10, const Vector4& _20, const Vector4& _30)
        : Mat4(_00, _10, _20, _30) {}

};
