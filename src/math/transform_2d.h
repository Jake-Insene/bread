#pragma once
#include "math/vec2.h"


/*
* Use a row major matrix2x3, rotation is handled in radians
*/
struct [[nodiscard]] Transform2D
{
    // It has the following layout
    // [0][0] [0][1] scale & rotation
    // [1][0] [1][1]
    // [2][0] [2][1] position
    Vector2 rows[3];

    static Transform2D with_rotation(const f32 rads);

    Transform2D()
    {
        rows[0] = Vector2(1, 0);
        rows[1] = Vector2(0, 1);
        rows[2] = Vector2(0, 0);
    }

    Transform2D(const Vector2 xx, const Vector2 yy, const Vector2 zz)
    {
        rows[0] = xx;
        rows[1] = yy;
        rows[2] = zz;
    }

    Vector2& operator[](usize index);
    const Vector2& operator[](usize index) const;

    [[nodiscard]] Transform2D operator*(const Transform2D& t) const;
    [[nodiscard]] Vector2 operator*(const Vector2& t) const;

    void set_position(Vector2 position);
    Vector2 get_position() const;

    void translate(Vector2 t);

    void set_scale(const Vector2& scale);
    Vector2 get_scale() const;

    void set_rotation(const f32 rads);
    [[nodiscard]] f32 get_rotation() const;
    void rotate(const f32 rads);

    [[nodiscard]] f32 determinant() const;
    Vector2 get_column(usize n) const;

    Transform2D inverse() const;
};
