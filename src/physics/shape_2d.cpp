#include "physics/shape_2d.h"


Shape2D Shape2D::make_box(const Vector2& size)
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

void Shape2D::translate(const Vector2& translation)
{
    vertices[0] += translation;
    vertices[1] += translation;
    vertices[2] += translation;
    vertices[3] += translation;
}
