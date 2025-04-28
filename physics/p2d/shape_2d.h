#pragma once
#include "math/vec2.h"

struct AABB
{
    Vector2 min;
    Vector2 max;
};

struct [[nodiscard]] Shape2D
{
    Vector2 position;
    Vector2 size;

    void set_position(const Vector2& new_pos)
    {
        position = new_pos;
    }

    Vector2 get_position() const
    {
        return position;
    }

    void translate(const Vector2& translation)
    {
        position += translation;
    }

    void set_size(const Vector2& new_size)
    {
        size = new_size;
    }

    Vector2 get_size() const
    {
        return size;
    }

    [[nodiscard]] bool intersect(const Shape2D& other)
    {
        return position.x < other.position.x + other.size.x &&
            position.x + size.x > other.position.x &&
            position.y > other.position.y - other.size.y &&
            position.y - size.y < other.position.y;
    }
};
