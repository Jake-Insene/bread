#pragma once
#include "core/types.h"
#include "debug/assertion.h"
#include "math/funcs.h"
#include "platform/instrinsics.h"



template <typename T>
union [[nodiscard]] Vector2T
{
    T comp[2]{};

    struct
    {
        T x;
        T y;
    };

    struct
    {
        T width;
        T height;
    };

    static constexpr Vector2T zero() { return Vector2T(0); }
    static constexpr Vector2T one() { return Vector2T(1); }
    
    static constexpr Vector2T up() { return Vector2T(0, 1); }
    static constexpr Vector2T down() { return Vector2T(0, -1); }
    
    static constexpr Vector2T right() { return Vector2T(1, 0); }
    static constexpr Vector2T left() { return Vector2T(-1, 0); }

    static constexpr T cross(const Vector2T& v1, const Vector2T& v2)
    {
		return v1.x * v2.y - v1.y * v2.x;
    }


    static constexpr Vector2T rotate_around_point(const Vector2T& vertice, const Vector2T& point, const T rot)
    {
        Vector2T rotated = Vector2T();

        const Vector2T direction = vertice - point;

		f32 s = math::sin(rot);
		f32 c = math::cos(rot);

		rotated.x = (direction.x * c) - (direction.y * s);
		rotated.y = (direction.x * s) + (direction.y * c);
		rotated += point;

		return rotated;
    }

    static constexpr Vector2T lerp(const Vector2T& v0, const Vector2T& v1, const T& t)
    {
        return math::lerp<Vector2T>(v0, v1, t);
    }
    
    constexpr Vector2T() : x(T(0)), y(T(0)) {};
    constexpr explicit Vector2T(const T _x, const T _y) : x(_x), y(_y) {}
    constexpr explicit Vector2T(const T v) : x(v), y(v) {}
    
    template<typename T2>
    constexpr explicit Vector2T(const Vector2T<T2>& v) : x(T(v.x)), y(T(v.y)) {}

    [[nodiscard]] constexpr T& operator[](const usize index)
    {
        DebugAssert(index < 2, "index can only be 0 or 1");
        return comp[index];
    }

    [[nodiscard]] constexpr const T& operator[](const usize index) const
    {
        DebugAssert(index < 2, "index can only be 0 or 1");
        return comp[index];
    }

    // Basic vector math
    constexpr Vector2T operator+(const Vector2T& v) const
    {
        return Vector2T{x + v.x, y + v.y};
    }
    
    constexpr Vector2T operator+(T v) const
    {
        return Vector2T{x + v, y + v};
    }

    constexpr Vector2T operator-(const Vector2T& v) const
    {
        return Vector2T{x - v.x, y - v.y};
    }
    
    constexpr Vector2T operator-(T v) const
    {
        return Vector2T{x - v, y - v};
    }

    constexpr Vector2T operator*(const Vector2T& v) const
    {
        return Vector2T{x * v.x, y * v.y};
    }

    constexpr Vector2T operator*(T v) const
    {
        return Vector2T{x * v, y * v};
    }

    constexpr Vector2T operator/(const Vector2T& v) const
    {
        return Vector2T{x / v.x, y / v.y};
    }

    constexpr Vector2T operator/(T v) const
    {
        return Vector2T{x / v, y / v};
    }

    constexpr Vector2T& operator+=(const Vector2T& v)
    {
        x += v.x;
        y += v.y;
        return *this;
    }

    constexpr Vector2T& operator-=(const Vector2T& v)
    {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    constexpr Vector2T& operator*=(const Vector2T& v)
    {
        x *= v.x;
        y *= v.y;
        return *this;
    }

    constexpr Vector2T& operator*=(T v)
    {
        x *= v;
        y *= v;
        return *this;
    }

    constexpr Vector2T& operator/=(const Vector2T& v)
    {
        x /= v.x;
        y /= v.y;
        return *this;
    }

    constexpr Vector2T& operator/=(T v)
    {
        x /= v;
        y /= v;
        return *this;
    }

    constexpr bool operator==(const Vector2T& v) const
    {
		return x == v.x && y == v.y;
    }
    
    constexpr T length() const
    {
        return math::sqrt(x * x + y * y);
    }

    constexpr Vector2T normalized() const
    {
        if(x == 0 && y == 0)
        {
            return Vector2T(0);
        }
        else if(x == 0)
        {
            return Vector2T(0, y / math::abs(y));
        }
        else if(y == 0)
        {
            return Vector2T(x / math::abs(x), 0);
        }


#if BREAD_ENABLE_INTRISICS
        T x1 = x;
        T y1 = y;
        PlatformIntricics::vecnormalize(x1, y1);
        return Vector2T(x1, y1);
#else
        Vector2T v = *this;
        const T len = (T)math::sqrt(x * x + y * y);
        if (len)
        {
            v.x /= len;
            v.y /= len;
        }

        return v;
#endif
    }
    
    constexpr void normalize()
    {
#if BREAD_ENABLE_INTRISICS
        PlatformIntricics::vecnormalize(x, y);
#else
        const T len = (T)math::sqrt(x * x + y * y);
        if(len)
        {
            x /= len;
            y /= len;
        }
#endif
    }

    constexpr Vector2T abs() const
    {
        return Vector2T(math::abs(x), math::abs(y));
    }
    
    constexpr T dot(const Vector2T& v) const
    {
        return x * v.x + y * v.y;
    }

    constexpr Vector2T normal() const
    {
		return Vector2T(-y, x);
    }
};

using Vector2 = Vector2T<f32>;
using Vector2I = Vector2T<i32>;

namespace io
{
struct Writer;
}

namespace fmt
{

template<typename T>
void format_custom(const io::Writer& writer, const T& v);

template<>
void format_custom<Vector2>(const io::Writer& writer, const Vector2& v);

template<>
void format_custom<Vector2I>(const io::Writer& writer, const Vector2I& v);

}



