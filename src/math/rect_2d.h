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
    constexpr explicit Rect2DT(const Vector2T<T>& pos, const Vector2T<T>& sz) : position(pos), size(sz) {}
    
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
        const Vector2T<T> min = position;
        const Vector2T<T> max = position + Vector2T(size.x, -size.y);

        const Vector2T<T> min_r = r.position;
        const Vector2T<T> max_r = r.position + Vector2T(r.size.x, -r.size.y);

        return min.x < max_r.x &&
            max.x > min_r.x &&
            min.y > max_r.y &&
            max.y < min_r.x;
    }

};

using Rect2D = Rect2DT<f32>;
using Rect2DI = Rect2DT<i32>;
