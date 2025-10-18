#include "physics/shape_2d.h"


Shape2D Shape2D::make_box(const Vector2& size)
{
    Shape2D box = {};
    box.vertices[0] = Vector2(-size.x, size.y);
    box.vertices[1] = Vector2(size.x, size.y);
    box.vertices[2] = Vector2(size.x, -size.y);
    box.vertices[3] = Vector2(-size.x, -size.y);
    return box;
}

void Shape2D::translate(const Vector2& translation)
{
    vertices[0] += translation;
    vertices[1] += translation;
    vertices[2] += translation;
    vertices[3] += translation;
}
