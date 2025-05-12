#pragma once
#include "core/types.h"
#include "core/macros.h"
#include "math/funcs.h"

template <typename T>
union [[nodiscard]] Vector4T
{
    T comp[4];
    struct
    {
        T x;
        T y;
        T z;
        T w;
    };
    
    static constexpr Vector4T zero() { return Vector4T(0); }
    static constexpr Vector4T one() { return Vector4T(1); }
    
    constexpr Vector4T() : x(0), y(0), z(0), w(0) {}
    constexpr Vector4T(const T _x, const T _y, const T _z, const T _w) : x(_x), y(_y), z(_z), w(_w) {}
    constexpr explicit Vector4T(const T v) : x(v), y(v), z(v), w(v) {}
    
    template<typename T2>
    constexpr explicit Vector4T(const Vector4T<T2>& v) : x(T2(v.x)), y(T2(v.y)), z(T2(v.z)), w(T2(v.w)) {}

    [[nodiscard]] constexpr T& operator[](const usize index)
    {
        DebugAssert(index < 4, "index can only be 0, 1, 2 or 3");
        return comp[index];
    }

    [[nodiscard]] constexpr const T& operator[](const usize index) const
    {
        DebugAssert(index < 4, "index can only be 0, 1, 2 or 3");
        return comp[index];
    }

    // Basic vector math
    constexpr Vector4T operator+(const Vector4T& v) const
    {
        return Vector4T{x + v.x, y + v.y, z + v.z, w + v.w};
    }

    constexpr Vector4T operator-(const Vector4T& v) const
    {
        return Vector4T{x - v.x, y - v.y, z - v.z, w - v.w};
    }

    constexpr Vector4T operator*(const Vector4T& v) const
    {
        return Vector4T{x * v.x, y * v.y, z * v.z, w * v.w};
    }

    constexpr Vector4T operator*(T v) const
    {
        return Vector4T{x * v, y * v, z * v, w * v};
    }

    constexpr Vector4T operator/(const Vector4T& v) const
    {
        return Vector4T{x / v.x, y / v.y, z / v.z, w / v.w};
    }

    constexpr Vector4T operator/(T v) const
    {
        return Vector4T{x / v, y / v, z / v, w / v};
    }

    constexpr Vector4T& operator+=(const Vector4T& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        w += v.w;
        return *this;
    }

    constexpr Vector4T& operator-=(const Vector4T& v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        w -= v.w;
        return *this;
    }

    constexpr Vector4T& operator*=(const Vector4T& v)
    {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        w *= v.w;
        return *this;
    }

    constexpr Vector4T& operator*=(T v)
    {
        x *= v;
        y *= v;
        z *= v;
        w *= v;
        return *this;
    }

    constexpr Vector4T& operator/=(const Vector4T& v)
    {
        x /= v.x;
        y /= v.y;
        z /= v.z;
        w /= v.w;
        return *this;
    }

    constexpr Vector4T& operator/=(T v)
    {
        x /= v;
        y /= v;
        z /= v;
        w /= v;
        return *this;
    }
    
    constexpr T length() const
    {
        return ::sqrt(x * x + y * y + z * z + w * w);
    }

    constexpr void normalize()
    {
        const T len = (T)sqrt(x * x + y * y + z * z + w * w);
        if(len)
        {
            x /= len;
            y /= len;
            z /= len;
            w /= len;
        }
    }
    
    constexpr T dot(const Vector4T& v) const
    {
        return x * v.x + y * v.y + z * v.z + w * v.w;
    }
};

using Vector4 = Vector4T<f32>;
