#pragma once
#include "math/vec2.h"

struct AABB
{
    Vector2 min;
    Vector2 max;
};

/*
* Use to represent the a shape that forms a body.
* Only 4 vertices for now(Quads).
*/
struct [[nodiscard]] Shape2D
{
    // 0 -> Top left
    // 1 -> Top right
    // 2 -> Bottom right
    // 3 -> Bottom left
    Vector2 vertices[4];

    static Shape2D make_box(const Vector2& size);

    void translate(const Vector2& translation);
};
