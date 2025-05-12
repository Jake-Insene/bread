#pragma once
#include "math/vec4.h"
#include "math/vec3.h"
#include "math/funcs.h"

// Mat4 is in row major
struct [[nodiscard]] Mat4
{
    Vector4 matrix[4];

    static constexpr Mat4 identity()
    {
        return Mat4
        {
            Vector4(1, 0, 0, 0),
            Vector4(0, 1, 0, 0),
            Vector4(0, 0, 1, 0),
            Vector4(0, 0, 0, 1)
        };
    }
    
    static constexpr Mat4 view(const Vector3& eye, const Vector3& target, const Vector3& up)
    {
        Vector3 z = target - eye;
        z.normalize();
        Vector3 x = Vector3::cross(up, z);
        x.normalize();
        Vector3 y = Vector3::cross(z, x);
        y.normalize();

        return Mat4
        {
            Vector4(x.x, x.y, x.z, -x.dot(eye)),
            Vector4(y.x, y.y, y.z, -y.dot(eye)),
            Vector4(z.x, z.y, z.z, -z.dot(eye)),
            Vector4(0, 0, 0, 1)
        };
    }

    static constexpr Mat4 translation(const Vector3& t)
    {
        return Mat4
        {
            Vector4(1, 0, 0, t.x),
            Vector4(0, 1, 0, t.y),
            Vector4(0, 0, 1, t.z),
            Vector4(0, 0, 0, 1)
        };
    }

    static constexpr Mat4 scaling(const Vector3& s)
    {
        return Mat4
        {
            Vector4(s.x, 0, 0, 0),
            Vector4(0, s.y, 0, 0),
            Vector4(0, 0, s.z, 0),
            Vector4(0, 0, 0, 1)
        };
    }

    static constexpr Mat4 rotation_z(const f32 degress)
    {
        const f32 r = math::rads(degress);
        const f32 c = math::cos(r);
        const f32 s = math::sin(r);

        return Mat4
        {
            Vector4(c, -s, 0, 0),
            Vector4(s, c, 0, 0),
            Vector4(0, 0, 1, 0),
            Vector4(0, 0, 0, 1)
        };
    }

    constexpr void look_at(const Vector3& eye, const Vector3& target, const Vector3& up)
    {
        Vector3 z = target - eye;
        z.normalize();
        Vector3 x = Vector3::cross(up, z);
        x.normalize();
        Vector3 y = Vector3::cross(z, x);
        y.normalize();
        
        matrix[0] = Vector4(x.x, x.y, x.z, -x.dot(eye));
        matrix[0] = Vector4(y.x, y.y, y.z, -y.dot(eye));
        matrix[0] = Vector4(z.x, z.y, z.z, -z.dot(eye));
        matrix[0] = Vector4(0, 0, 0, 1);
    }
    
    constexpr void translate(const Vector3& t)
    {
        matrix[3] += Vector4(t.x, t.y, t.z, 0);
    }
    
    constexpr Mat4()
    {
        ::new(this) Mat4(identity());
    }
    
    constexpr Mat4(const Vector4& _00, const Vector4& _10, const Vector4& _20, const Vector4& _30)
    {
        matrix[0] = _00;
        matrix[1] = _10;
        matrix[2] = _20;
        matrix[3] = _30;
    }

    constexpr Mat4(const Mat4& m)
    {
		matrix[0] = m[0];
		matrix[1] = m[1];
		matrix[2] = m[2];
		matrix[3] = m[3];
    }

    constexpr Mat4& operator=(const Mat4& m)
    {
        matrix[0] = m[0];
        matrix[1] = m[1];
        matrix[2] = m[2];
        matrix[3] = m[3];
        return *this;
    }

    constexpr Vector4& operator[](usize index)
    {
        DebugAssert(index < 4, "index can only be 0, 1, 2 or 3");
        return matrix[index];
    }

    constexpr const Vector4& operator[](usize index) const
    {
        DebugAssert(index < 4, "index can only be 0, 1, 2 or 3");
        return matrix[index];
    }
    
    constexpr Mat4 operator*(const Mat4& mat)
    {
        Mat4 output = identity();
        
        for (usize i = 0; i < 4; ++i)
        {
            for (usize j = 0; j < 4; ++j)
            {
                output[i][j] =
                    matrix[i][0] * mat[0][j] +
                    matrix[i][1] * mat[1][j] +
                    matrix[i][2] * mat[2][j] +
                    matrix[i][3] * mat[3][j];
            }
        }
        
        return output;
    }
    
    constexpr Mat4& operator*=(const Mat4& mat)
    {
        for (i32 i = 0; i < 4; ++i)
        {
            for (i32 j = 0; j < 4; ++j)
            {
                matrix[i][j] =
                    matrix[i][0] * mat[0][j] +
                    matrix[i][1] * mat[1][j] +
                    matrix[i][2] * mat[2][j] +
                    matrix[i][3] * mat[3][j];
            }
        }
        
        return *this;
    }

    constexpr Mat4 transposing()
    {
        Mat4 transposed{};

        for (i32 i = 0; i < 4; ++i)
        {
            for (i32 j = 0; j < 4; ++j)
            {
                transposed[j][i] = matrix[i][j];  // Swap row and column
            }
        }

        return transposed;
    }

    constexpr void transpose()
    {
        Mat4 transposed{};

        for (i32 i = 0; i < 4; ++i)
        {
            for (i32 j = 0; j < 4; ++j)
            {
                transposed[j][i] = matrix[i][j];  // Swap row and column
            }
        }

		::new(this) Mat4(transposed);
    }
    
    constexpr Mat4 inverse()
    {
        Mat4 m = *this;
        Mat4 out{};
        f32 cof00 = m[1][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) - 
                  m[1][2] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) +
                  m[1][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]);

        f32 cof01 = -(m[1][0] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) - 
                        m[1][2] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) +
                        m[1][3] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]));
    
        f32 cof02 = m[1][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) - 
                    m[1][1] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) +
                    m[1][3] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]);
    
        f32 cof03 = -(m[1][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) - 
                        m[1][1] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]) +
                        m[1][2] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]));
    
        f32 cof10 = -(m[0][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) - 
                        m[0][2] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) +
                        m[0][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]));
    
        f32 cof11 = m[0][0] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) - 
                    m[0][2] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) +
                    m[0][3] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]);
    
        f32 cof12 = -(m[0][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) - 
                        m[0][1] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) +
                        m[0][3] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]));
    
        f32 cof13 = m[0][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) - 
                    m[0][1] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]) +
                    m[0][2] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]);
    
        f32 cof20 = m[0][1] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) - 
                    m[0][2] * (m[1][1] * m[3][3] - m[1][3] * m[3][1]) +
                    m[0][3] * (m[1][1] * m[3][2] - m[1][2] * m[3][1]);
    
        f32 cof21 = -(m[0][0] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) - 
                        m[0][2] * (m[1][0] * m[3][3] - m[1][3] * m[3][0]) +
                        m[0][3] * (m[1][0] * m[3][2] - m[1][2] * m[3][0]));
    
        f32 cof22 = m[0][0] * (m[1][1] * m[3][3] - m[1][3] * m[3][1]) - 
                    m[0][1] * (m[1][0] * m[3][3] - m[1][3] * m[3][0]) +
                    m[0][3] * (m[1][0] * m[3][1] - m[1][1] * m[3][0]);
    
        f32 cof23 = -(m[0][0] * (m[1][1] * m[3][2] - m[1][2] * m[3][1]) - 
                        m[0][1] * (m[1][0] * m[3][2] - m[1][2] * m[3][0]) +
                        m[0][2] * (m[1][0] * m[3][1] - m[1][1] * m[3][0]));
    
        f32 cof30 = -(m[0][1] * (m[1][2] * m[2][3] - m[1][3] * m[2][2]) - 
                        m[0][2] * (m[1][1] * m[2][3] - m[1][3] * m[2][1]) +
                        m[0][3] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]));
    
        f32 cof31 = m[0][0] * (m[1][2] * m[2][3] - m[1][3] * m[2][2]) - 
                    m[0][2] * (m[1][0] * m[2][3] - m[1][3] * m[2][0]) +
                    m[0][3] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]);
    
        f32 cof32 = -(m[0][0] * (m[1][1] * m[2][3] - m[1][3] * m[2][1]) - 
                        m[0][1] * (m[1][0] * m[2][3] - m[1][3] * m[2][0]) +
                        m[0][3] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]));
    
        f32 cof33 = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) - 
                    m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
                    m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
    
        // Determinant of the matrix
        f32 det = m[0][0] * cof00 + m[0][1] * cof01 + m[0][2] * cof02 + m[0][3] * cof03;
        if (det == 0.0f)
        {
            return identity();
        }
    
        // Compute the inverse by dividing the adjugate (cofactor matrix transpose) by determinant
        out[0][0] = cof00 / det; out[0][1] = cof10 / det; out[0][2] = cof20 / det; out[0][3] = cof30 / det;
        out[1][0] = cof01 / det; out[1][1] = cof11 / det; out[1][2] = cof21 / det; out[1][3] = cof31 / det;
        out[2][0] = cof02 / det; out[2][1] = cof12 / det; out[2][2] = cof22 / det; out[2][3] = cof32 / det;
        out[3][0] = cof03 / det; out[3][1] = cof13 / det; out[3][2] = cof23 / det; out[3][3] = cof33 / det;
    
        return out;
    }


};
