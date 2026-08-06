#pragma once
#include "math/vec2.h"


/*
* Bottom Left rectangle.
*/
template<typename T>
struct [[nodiscard]] Rect2DT
{
    using Type = T;
    using VectorType = Vector2T<Type>;

    VectorType position;
    VectorType size;
    
    static constexpr Rect2DT zero() { return Rect2DT(0, 0, 0, 0); }
    static constexpr Rect2DT one() { return Rect2DT(1, 1, 1, 1); }
    
    constexpr Rect2DT() = default;
    constexpr explicit Rect2DT(const Type x, const Type y, const Type w, const Type h) : position(x, y), size(w, h) {}
    constexpr explicit Rect2DT(const VectorType& pos, const VectorType& sz) : position(pos), size(sz) {}
    
    [[nodiscard]] constexpr bool operator==(const Rect2DT& other) const
    {
		return position == other.position && size == other.size;
    }

    [[nodiscard]] constexpr bool contains(const VectorType& point)
    {
        if(point.x < position.x || point.y < position.y)
        {
            return false;
        }
        
        if(point.x > (position.x + size.width) || point.y > (position.y + size.height))
        {
            return false;
        }

        return true;
    }

    [[nodiscard]] constexpr bool is_inside(const Rect2DT& other_rect)
    {
        return other_rect.position.x >= position.x &&
               other_rect.position.x + other_rect.size.width <= position.x + size.width &&
               other_rect.position.y <= position.y + size.height &&
               other_rect.position.y - other_rect.size.height <= position.y + size.height;
    }

    [[nodiscard]] constexpr bool is_colliding(const Rect2DT& other_rect)
    {
        const VectorType min = position;
        const VectorType max = position + VectorType(size.x, size.y);

        const VectorType min_r = other_rect.position;
        const VectorType max_r = other_rect.position + VectorType(other_rect.size.x, other_rect.size.y);

        return min.x < max_r.x &&
            max.x > min_r.x &&
            min.y > max_r.y &&
            max.y < min_r.x;
    }

    constexpr VectorType center() const
    {
        return position + (size / Type(2));
    }

    constexpr void set_center(const VectorType& new_center)
    {
        position.x = new_center.x - size.x/T(2);
        position.y = new_center.y - size.y/T(2);
    }
};

using Rect2D = Rect2DT<f32>;
using Rect2DI = Rect2DT<i32>;
