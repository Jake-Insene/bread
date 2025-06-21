#pragma once
#include "math/vec2.h"


template<typename T>
struct [[nodiscard]] Rect2DT
{
    Vector2T<T> position = Vector2T<T>(0, 0);
    Vector2T<T> size = Vector2T<T>(0, 0);
    
    static constexpr Rect2DT zero() { return Rect2DT(0, 0, 0, 0); }
    static constexpr Rect2DT one() { return Rect2DT(1, 1, 1, 1); }
    
    constexpr Rect2DT() = default;
    constexpr explicit Rect2DT(const T x, const T y, const T w, const T h) : position(x, y), size(w, h) {}
    constexpr explicit Rect2DT(const Vector2T<T>& p, const Vector2T<T>& s) : position(p), size(s) {}
    
    [[nodiscard]] constexpr bool point_is_in(const Vector2T<T>& p)
    {
        if(p.x < position.x || p.y > position.y)
        {
            return false;
        }
        
        if(p.x > (position.x + size.width) || p.y < (position.y - size.height))
        {
            return false;
        }

        return true;
    }

    [[nodiscard]] constexpr bool is_inside(const Rect2DT& r)
    {
        return r.position.x >= position.x &&
               r.position.x + r.size.width <= position.x + size.width &&
               r.position.y <= position.y - size.height &&
               r.position.y - r.size.height <= position.y - size.height;
    }

    [[nodiscard]] constexpr bool is_colliding(const Rect2DT& r)
    {
        return position.x < r.position.x + r.size.width &&
               position.x + size.width > r.position.x &&
               position.y < r.position.y - r.size.height &&
               position.y - size.height <= r.position.y;
    }

};

using Rect2D = Rect2DT<f32>;
using Rect2DI = Rect2DT<i32>;
