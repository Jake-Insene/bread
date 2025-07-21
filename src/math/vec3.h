#pragma once
#include "core/types.h"
#include "debug/debug.h"
#include "debug/assertion.h"
#include "math/funcs.h"

template <typename T>
union [[nodiscard]] Vector3T
{
    T comp[3];
    struct
    {
        T x;
        T y;
        T z;
    };
    
    static constexpr Vector3T zero() { return Vector3T(0); }
    static constexpr Vector3T one() { return Vector3T(1); }
    
    static constexpr Vector3T up() { return Vector3T(0, 1, 0); }
    static constexpr Vector3T down() { return Vector3T(0, -1, 0); }
    
    static constexpr Vector3T right() { return Vector3T(1, 0, 0); }
    static constexpr Vector3T left() { return Vector3T(-1, 0, 0); }
    
    static constexpr Vector3T cross(const Vector3T& v1, const Vector3T& v2)
    {
        return Vector3T
        {
            (v1.y * v2.z) - (v1.z * v2.y),
            (v1.z * v2.x) - (v1.x * v2.z),
            (v1.x * v2.y) - (v1.y * v2.x)
        };
    }
    
    constexpr Vector3T() : x(0), y(0), z(0) {}
    constexpr explicit Vector3T(const T _x, const T _y, const T _z) : x(_x), y(_y), z(_z) {}
    constexpr explicit Vector3T(const T v) : x(v), y(v), z(v) {}
    
    template<typename T2>
    constexpr explicit Vector3T(const Vector3T<T2>& v) : x(T2(v.x)), y(T2(v.y)), z(T2(v.z)) {}

    [[nodiscard]] constexpr T& operator[](const usize index)
    {
        DebugAssert(index < 3, "index can only be 0, 1 or 2");
        return comp[index];
    }

    [[nodiscard]] constexpr const T& operator[](const usize index) const
    {
        DebugAssert(index < 3, "index can only be 0, 1 or 2");
        return comp[index];
    }

    // Basic vector math
    constexpr Vector3T operator+(const Vector3T& v) const
    {
        return Vector3T{x + v.x, y + v.y, z + v.z};
    }

    constexpr Vector3T operator-(const Vector3T& v) const
    {
        return Vector3T{x - v.x, y - v.y, z - v.z};
    }

    constexpr Vector3T operator*(const Vector3T& v) const
    {
        return Vector3T{x * v.x, y * v.y, z * v.z};
    }

    constexpr Vector3T operator*(T v) const
    {
        return Vector3T{x * v, y * v, z * v};
    }

    constexpr Vector3T operator/(const Vector3T& v) const
    {
        return Vector3T{x / v.x, y / v.y, z / v.z};
    }

    constexpr Vector3T operator/(T v) const
    {
        return Vector3T{x / v, y / v, z / v};
    }

    constexpr Vector3T& operator+=(const Vector3T& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    constexpr Vector3T& operator-=(const Vector3T& v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    constexpr Vector3T& operator*=(const Vector3T& v)
    {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        return *this;
    }

    constexpr Vector3T& operator*=(T v)
    {
        x *= v;
        y *= v;
        z *= v;
        return *this;
    }

    constexpr Vector3T& operator/=(const Vector3T& v)
    {
        x /= v.x;
        y /= v.y;
        z /= v.z;
        return *this;
    }

    constexpr Vector3T& operator/=(T v)
    {
        x /= v;
        y /= v;
        z /= v;
        return *this;
    }

    constexpr bool operator==(const Vector3T& v)
    {
        return x == v.x && y == v.y && z == v.z;
    }
    
    constexpr T length() const
    {
        return math::sqrt(x * x + y * y + z * z);
    }

    constexpr void normalize()
    {
        const T len = (T)math::sqrt(x * x + y * y + z * z);
        if(len)
        {
            x /= len;
            y /= len;
            z /= len;
        }
    }
    
    constexpr T dot(const Vector3T& v) const
    {
        return x * v.x + y * v.y + z * v.z;
    }
};

using Vector3 = Vector3T<f32>;
