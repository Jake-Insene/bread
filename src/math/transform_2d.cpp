#include "math/transform_2d.h"


#include "math/funcs.h"


Transform2D Transform2D::with_rotation(const f32 rads)
{
    f32 s, c;
    math::sincos(s, c, rads);
    return Transform2D(
        Vector2(c, -s),
        Vector2(s, c),
        Vector2()
    );
}

Vector2& Transform2D::operator[](usize index)
{
    DebugAssert(index < 3, "index can only be 0, 1 or 2");
    return rows[index];
}

const Vector2& Transform2D::operator[](usize index) const
{
    DebugAssert(index < 3, "index can only be 0, 1 or 2");
    return rows[index];
}


[[nodiscard]] Transform2D Transform2D::operator*(const Transform2D& t) const
{
    const Vector2 new_pos
    {
        (rows[0][0] * t[2][0] + rows[0][1] * t[2][1]) + rows[2][0],
        (rows[1][0] * t[2][0] + rows[1][1] * t[2][1]) + rows[2][1],
    };

    return Transform2D(
        Vector2(rows[0][0] * t[0][0] + rows[0][1] * t[1][0], rows[0][0] * t[0][1] + rows[0][1] * t[1][1]),
        Vector2(rows[1][0] * t[0][0] + rows[1][1] * t[1][0], rows[1][0] * t[0][1] + rows[1][1] * t[1][1]),
        new_pos
    );
}


Vector2 Transform2D::operator*(const Vector2& t) const
{
    const Vector2 new_pos
    {
        (rows[0][0] * t.x + rows[0][1] * t.y) + rows[2].x,
        (rows[1][0] * t.x + rows[1][1] * t.y) + rows[2].y,
    };

    return new_pos;
}


void Transform2D::set_position(Vector2 position)
{
    rows[2] = position;
}

Vector2 Transform2D::get_position() const
{
    return rows[2];
}

void Transform2D::translate(Vector2 t)
{
    rows[2] += t;
}

void Transform2D::set_scale(const Vector2& scale)
{
    rows[0].normalize();
    rows[1].normalize();
    rows[0] *= scale.x;
    rows[1] *= scale.y;
}

Vector2 Transform2D::get_scale() const
{
    return Vector2(rows[0].length(), rows[1].length());
}

void Transform2D::set_rotation(const f32 rads)
{
    f32 s, c;
    math::sincos(s, c, rads);
    rows[0] = Vector2(c, -s);
    rows[1] = Vector2(s, c);
}

f32 Transform2D::get_rotation() const
{
    return math::atan2(rows[1].x, rows[0].x);
}

void Transform2D::rotate(const f32 rads)
{
    *this = *this * Transform2D::with_rotation(rads);
}

f32 Transform2D::determinant() const
{
    return rows[0].x * rows[1].y - rows[0].y * rows[1].x;
}

Vector2 Transform2D::get_column(usize n) const
{
    if (n == 0)
    {
        return Vector2(rows[0].x, rows[1].x);
    }
    else
    {
        return Vector2(rows[0].y, rows[1].y);
    }
}

Transform2D Transform2D::inverse() const
{
    const f32 det = determinant();
    if (det < 0.0001f)
    {
        return Transform2D();
    }

    const f32 inv_det = 1.f / det;

    // 2x2 inverse
    Vector2 i0(rows[1].y * inv_det, -rows[0].y * inv_det);
    Vector2 i1(-rows[1].x * inv_det, rows[0].x * inv_det);

    // Inverse translation
    Vector2 pos = rows[2];
    Vector2 i2(
        -(pos.x * i0.x + pos.y * i1.x),
        -(pos.x * i0.y + pos.y * i1.y)
    );

    return Transform2D(i0, i1, i2);
}
