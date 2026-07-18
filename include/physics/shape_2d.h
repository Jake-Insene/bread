#pragma once
#include "collections/array.h"
#include "math/vec2.h"


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
    Array<Vector2> vertices;

    static Shape2D make_box(Mem::Allocator* allocator, const Vector2& size);

    void destroy();

    void translate(const Vector2& translation);
};
