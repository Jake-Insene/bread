#include "physics/shape_2d.h"



Shape2D Shape2D::make_box(Mem::Allocator* allocator, const Vector2& size)
{
    Shape2D box = 
    {
        .vertices = Array<Vector2>::with_size(allocator, 4),
    };
    (void)box.vertices.add(Vector2(-size.x, size.y));
    (void)box.vertices.add(Vector2(size.x, size.y));
    (void)box.vertices.add(Vector2(size.x, -size.y));
    (void)box.vertices.add(Vector2(-size.x, -size.y));
    return box;
}


void Shape2D::destroy()
{
    vertices.destroy();
}

void Shape2D::translate(const Vector2& translation)
{
    for(Vector2& v : vertices.iter())
    {
        v += translation;
    }
}
