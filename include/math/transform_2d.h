#pragma once
#include "math/vec2.h"


/*
* Use a row major matrix2x3, rotation is handled in radians
*/
template <typename T>
requires(Core::IsArithmetic<T>)
struct [[nodiscard]] Transform2DT
{
    using Type = T;
    using VectorType = Vector2T<Type>;

    // It has the following layout
    // [0][0] [0][1] scale & rotation
    // [1][0] [1][1]
    // [2][0] [2][1] position
    VectorType rows[3];

    static Transform2DT with_rotation(const Type rads)
    {
        f32 sin = 0;
        f32 cos = 0;
        Math::sincos(&sin, &cos, rads);
        return Transform2DT(
            Vector2(cos, -sin),
            Vector2(sin, cos),
            Vector2()
        );
    }

    static Transform2DT with_position(const VectorType& position)
    {
        return Transform2DT(
            Vector2(1, 0),
            Vector2(0, 1),
            position
        );
    }

    constexpr Transform2DT()
    {
        rows[0] = VectorType(1, 0);
        rows[1] = VectorType(0, 1);
        rows[2] = VectorType(0, 0);
    }

    constexpr Transform2DT(const VectorType xx, const VectorType yy, const VectorType zz)
    {
        rows[0] = xx;
        rows[1] = yy;
        rows[2] = zz;
    }

    template<typename Self>
    constexpr auto& operator[](this Self& self, usize index)
    {
        DebugAssert(index < 3, "index can only be 0, 1 or 2");
        return self.rows[index];
    }

    [[nodiscard]] Transform2DT operator*(const Transform2DT& transform) const
    {
        const VectorType new_pos = VectorType(
            ((rows[0][0] * transform[2][0]) + (rows[0][1] * transform[2][1])) + rows[2][0],
            ((rows[1][0] * transform[2][0]) + (rows[1][1] * transform[2][1])) + rows[2][1]
        );

        return Transform2DT(
            VectorType(
                (rows[0][0] * transform[0][0]) + (rows[0][1] * transform[1][0]),
                (rows[0][0] * transform[0][1]) + (rows[0][1] * transform[1][1])
            ),
            VectorType(
                (rows[1][0] * transform[0][0]) + (rows[1][1] * transform[1][0]),
                (rows[1][0] * transform[0][1]) + (rows[1][1] * transform[1][1])
            ),
            new_pos
        );
    }

    [[nodiscard]] VectorType operator*(const VectorType& position) const
    {
        const Vector2 new_pos
        {
            ((rows[0][0] * position.x) + (rows[0][1] * position.y)) + rows[2].x,
            ((rows[1][0] * position.x) + (rows[1][1] * position.y)) + rows[2].y,
        };

        return new_pos;
    }

    void set_position(const VectorType& new_position)
    {
        rows[2] = new_position;
    }

    VectorType get_position() const
    {
        return rows[2];
    }

    void translate(const VectorType& translation)
    {
        rows[2] += translation;
    }

    void set_scale(const VectorType& scale)
    {
        rows[0].normalize();
        rows[1].normalize();
        rows[0] *= scale.x;
        rows[1] *= scale.y;
    }

    VectorType get_scale() const
    {
        return Vector2(rows[0].length(), rows[1].length());
    }

    void set_rotation(const Type rads)
    {
        f32 sin = 0;
        f32 cos = 0;
        Math::sincos(&sin, &cos, rads);
        rows[0] = Vector2(cos, -sin);
        rows[1] = Vector2(sin, cos);
    }

    [[nodiscard]] Type get_rotation() const
    {
        return Math::atan2(rows[1].x, rows[0].x);
    }

    void rotate(const Type rads)
    {
        *this = *this * Transform2DT::with_rotation(rads);
    }

    [[nodiscard]] Type determinant() const
    {
        return (rows[0].x * rows[1].y) - (rows[0].y * rows[1].x);
    }

    VectorType get_column(usize n) const
    {
        DebugAssert(n < 2, "only 0, 1 are valid columns");
        if (n == 0)
        {
            return VectorType(rows[0].x, rows[1].x);
        }

        return VectorType(rows[0].y, rows[1].y);
    }

    Transform2DT inverse() const
    {
        const Type det = determinant();
        if (det < Type(0.0001))
        {
            return Transform2D();
        }

        const Type inv_det = Type(1.0) / det;

        // 2x2 inverse
        VectorType i0 = VectorType(rows[1].y * inv_det, -rows[0].y * inv_det);
        VectorType i1 = VectorType(-rows[1].x * inv_det, rows[0].x * inv_det);

        // Inverse translation
        VectorType pos = rows[2];
        VectorType i2 = VectorType(
            -((pos.x * i0.x) + (pos.y * i1.x)),
            -((pos.x * i0.y) + (pos.y * i1.y))
        );

        return Transform2DT(i0, i1, i2);
    }
};

using Transform2D = Transform2DT<f32>;
