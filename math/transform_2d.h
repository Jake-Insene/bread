#pragma once
#include "math/funcs.h"
#include "math/vec2.h"

// Use a row major matrix2x3
struct [[nodiscard]] Transform2D
{
    // It has the following layout
    // [0][0] [0][1] scale & rotation
    // [1][0] [1][1]
    Vector2 rows[2];
    // [2][0] [2][1] position
    Vector2 origin;
    
    explicit constexpr Transform2D()
    {
        rows[0] = Vector2(1, 0);
        rows[1] = Vector2(0, 1);
        origin = Vector2(0, 0);
    }
    
    explicit constexpr Transform2D(Vector2 xx, Vector2 yy, Vector2 zz)
    {
        rows[0] = xx;
        rows[1] = yy;
        origin = zz;
    }
    
    constexpr Vector2& operator[](usize index)
    {
        DebugAssert(index < 3, "index can only be 0, 1 or 2");
        return rows[index];
    }

    constexpr const Vector2& operator[](usize index) const
    {
        DebugAssert(index < 3, "index can only be 0, 1 or 2");
        return rows[index];
    }

    [[nodiscard]] constexpr Transform2D operator*(const Transform2D& t) const
    {
        const Vector2 new_pos
        {
            (rows[0][0] * t.origin.x + rows[0][1] * t.origin.y) + origin.x,
            (rows[1][0] * t.origin.x + rows[1][1] * t.origin.y) + origin.y,
        };

        return Transform2D
        {
            Vector2(rows[0][0] * t[0][0] + rows[0][1] * t[1][0], rows[0][0] * t[0][1] + rows[0][1] * t[1][1]),
            Vector2(rows[1][0] * t[0][0] + rows[1][1] * t[1][0], rows[1][0] * t[0][1] + rows[1][1] * t[1][1]),
            new_pos,
        };
    }
    
    constexpr void set_position(Vector2 position)
    {
        rows[2] = position;
    }
    
    constexpr Vector2 get_position() const
    {
        return origin;
    }
    
    constexpr void translate(Vector2 t)
    {
        origin += t;
    }
    
    constexpr void set_scale(const Vector2& scale)
    {
        rows[0].normalize();
        rows[1].normalize();
        rows[0] *= scale.x;
        rows[1] *= scale.y;
    }
    
    constexpr Vector2 get_scale() const
    {
        return Vector2{ rows[0].length(), rows[1].length() };
    }
    
    constexpr void set_rotation(const f32 rads)
    {
        const Vector2 scale = get_scale();
        const f32 c = math::cos(rads);
        const f32 s = math::sin(rads);
        rows[0][0] = c;
        rows[0][1] = -s;
        rows[1][0] = s;
        rows[1][1] = c;
        set_scale(scale);
    }
    
    [[nodiscard]] constexpr f32 get_rotation() const
    {
        return math::atan2(rows[0].y, rows[0].x);
    }
    
    [[nodiscard]] constexpr f32 determinant() const
    {
        return rows[0].x * rows[1].y - rows[0].y * rows[1].x;
    }
};
