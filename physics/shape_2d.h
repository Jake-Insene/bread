#pragma once
#include "math/vec2.h"

struct AABB
{
    Vector2 min;
    Vector2 max;
};

struct [[nodiscard]] Shape2D
{
    // 0 -> Top left
    // 1 -> Top right
    // 2 -> Bottom right
    // 3 -> Bottom left
    Vector2 vertices[4];

    static constexpr Shape2D make_box(Vector2 size)
    {
        return Shape2D
        {
            .vertices =
            {
                Vector2(-size.x, size.y),
                Vector2(size.x, size.y),
                Vector2(size.x, -size.y),
                Vector2(-size.x, -size.y),
            }
        };
    }

    void translate(Vector2 translation)
    {
        vertices[0] += translation;
        vertices[1] += translation;
        vertices[2] += translation;
        vertices[3] += translation;
    }

    void set_size(const Vector2& new_size)
    {
        vertices[0] = Vector2(-new_size.x, new_size.y);
        vertices[1] = Vector2(new_size.x, new_size.y);
        vertices[2] = Vector2(new_size.x, -new_size.y);
        vertices[3] = Vector2(-new_size.x, -new_size.y);
    }

    Vector2 get_size() const
    {
        return (vertices[2] - vertices[0]).abs();
    }

    AABB get_aabb() const { return AABB{ vertices[0], vertices[2] }; }

    Vector2 get_center() const
    {
        return vertices[0] + Vector2(get_size().x/2.f, -get_size().y/2.f);
    }

    [[nodiscard]] bool intersect(const Shape2D& other) const
    {
        return vertices[0].x < other.vertices[2].x &&
            vertices[2].x > other.vertices[0].x &&
            vertices[0].y > other.vertices[2].y &&
            vertices[2].y < other.vertices[0].y;
    }
};
