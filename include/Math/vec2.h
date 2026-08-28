#pragma once
#include "Core/Header.hpp"
#include "Debug/Assertion.hpp"
#include "fmt/fmt.h"
#include "Math/funcs.h"
#include "Platform/instrinsics.h"


template <typename T>
requires(Core::IsArithmetic<T>)
union [[nodiscard]] Vector2T
{
    using Type = T;

    Type comp[2];

    struct
    {
        Type x;
        Type y;
    };

    struct
    {
        Type width;
        Type height;
    };

    static constexpr Vector2T zero() { return Vector2T(0); }
    static constexpr Vector2T one() { return Vector2T(1); }
    
    static constexpr Vector2T up() { return Vector2T(0, 1); }
    static constexpr Vector2T down() { return Vector2T(0, -1); }
    
    static constexpr Vector2T right() { return Vector2T(1, 0); }
    static constexpr Vector2T left() { return Vector2T(-1, 0); }

    static constexpr Type cross(const Vector2T& v1, const Vector2T& v2)
    {
		return (v1.x * v2.y) - (v1.y * v2.x);
    }

    static constexpr Vector2T rotate_around_point(const Vector2T& vertice, const Vector2T& point, const Type rot)
    {
        Vector2T rotated = Vector2T();

        const Vector2T direction = vertice - point;

		f32 s = Math::sin(rot);
		f32 c = Math::cos(rot);

		rotated.x = (direction.x * c) - (direction.y * s);
		rotated.y = (direction.x * s) + (direction.y * c);
		rotated += point;

		return rotated;
    }

    static constexpr Vector2T lerp(const Vector2T& v0, const Vector2T& v1, const Type& t)
    {
        return Math::lerp<Vector2T>(v0, v1, t);
    }
    
    constexpr Vector2T() : x(Type(0)), y(Type(0)) {};
    constexpr explicit Vector2T(const Type _x, const Type _y) : x(_x), y(_y) {}
    constexpr explicit Vector2T(const Type v) : x(v), y(v) {}
    
    template<typename T2>
    constexpr explicit Vector2T(const Vector2T<T2>& v) : x(Type(v.x)), y(Type(v.y)) {}

    [[nodiscard]] constexpr Type& operator[](const usize index)
    {
        DebugAssert(index < 2, "index can only be 0 or 1");
        return comp[index];
    }

    [[nodiscard]] constexpr const Type& operator[](const usize index) const
    {
        DebugAssert(index < 2, "index can only be 0 or 1");
        return comp[index];
    }

    // Basic vector math
    constexpr Vector2T operator+(const Vector2T& v) const
    {
        return Vector2T{x + v.x, y + v.y};
    }
    
    constexpr Vector2T operator+(Type v) const
    {
        return Vector2T{x + v, y + v};
    }

    constexpr Vector2T operator-(const Vector2T& v) const
    {
        return Vector2T{x - v.x, y - v.y};
    }
    
    constexpr Vector2T operator-(Type v) const
    {
        return Vector2T{x - v, y - v};
    }

    constexpr Vector2T operator*(const Vector2T& v) const
    {
        return Vector2T{x * v.x, y * v.y};
    }

    constexpr Vector2T operator*(Type v) const
    {
        return Vector2T{x * v, y * v};
    }

    constexpr Vector2T operator/(const Vector2T& v) const
    {
        return Vector2T{x / v.x, y / v.y};
    }

    constexpr Vector2T operator/(Type v) const
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

    constexpr Vector2T& operator*=(Type v)
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

    constexpr Vector2T& operator/=(Type v)
    {
        x /= v;
        y /= v;
        return *this;
    }

    constexpr bool operator==(const Vector2T& v) const
    {
		return x == v.x && y == v.y;
    }
    
    constexpr Type length() const
    {
        return Math::sqrt(dot(*this));
    }

    constexpr Vector2T normalized() const
    {
        if(x == 0 && y == 0)
        {
            return Vector2T(0);
        }
        if(x == 0)
        {
            return Vector2T(0, y / Math::abs(y));
        }
        if(y == 0)
        {
            return Vector2T(x / Math::abs(x), 0);
        }

#if BREAD_ENABLE_INTRISICS
        if !consteval
        {
            Type x1 = x;
            Type y1 = y;
            PlatformIntrisics::vecnormalize(x1, y1);
            return Vector2T(x1, y1);
        }
#endif
        Vector2T v = *this;
        const Type len = length();
        if(len)
        {
            v.x /= len;
            v.y /= len;
        }

        return v;
    }
    
    constexpr void normalize()
    {
        if(x == 0 && y == 0)
        {
            return;
        }
#if BREAD_ENABLE_INTRISICS
        if !consteval
        {
            PlatformIntrisics::vecnormalize(x, y);
            return;
        }
#endif
        const Type len = length();
        if(len)
        {
            x /= len;
            y /= len;
        }
    }

    constexpr Type dot(const Vector2T& v) const
    {
        return (x * v.x) + (y * v.y);
    }

    constexpr Vector2T normal() const
    {
		return Vector2T(-y, x);
    }
};

using Vector2 = Vector2T<f32>;
using Vector2I = Vector2T<i32>;
using Vector2U = Vector2T<u32>;

namespace Format
{

template<typename T>
struct Formatter<Vector2T<T>>
{
	static void format_custom(const IO::Writer& writer, const Vector2T<T>& vec)
    {
        format<false>(writer, "({}, {})", vec.x, vec.y);
    }
};

}



