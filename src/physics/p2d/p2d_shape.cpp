#include "physics/p2d/p2d_shape.h"


P2DShape P2DShape::from_shape_2d(const Shape2D& shape)
{
    P2DShape pshape = {};
    
    pshape.vertices[0] =shape.vertices[0];
    pshape.vertices[1] = shape.vertices[1];
    pshape.vertices[2] =shape.vertices[2];
    pshape.vertices[3] = shape.vertices[3];

    pshape._calc_aabb();
    pshape._calc_normals();
    pshape._calc_area();
    pshape._calc_centroid();
    
    return pshape;
}

void P2DShape::apply_transform(const Transform2D& transform)
{
    vertices[0] = transform * vertices[0];
    vertices[1] = transform * vertices[1];
    vertices[2] = transform * vertices[2];
    vertices[3] = transform * vertices[3];

    centroid += transform.get_position();

    // The transformation could rotate the vertices
    _calc_aabb();
    _calc_normals();
}

void P2DShape::translate(const Vector2& translation)
{
    vertices[0] += translation;
    vertices[1] += translation;
    vertices[2] += translation;
    vertices[3] += translation;

    centroid += translation;
 
    _calc_aabb();
}

void P2DShape::rotate(const f32 r)
{
    for (usize i = 0; i < 4; i++)
    {
        vertices[i] = Vector2::rotate_around_point(vertices[i], centroid, r);
    }

    _calc_normals();
    _calc_aabb();
}

f32 P2DShape::get_area() const
{
    return area;
}

Vector2 P2DShape::get_centroid() const
{
    return centroid;
}

f32 P2DShape::calculate_inertia(f32 mass) const
{
    f32 inertia = 0.f;
    const f32 mass_per_triangle_face = mass / 4.f;
    for(usize i = 0; i < 4; i++)
    {
        const Vector2 center_to_vertice = vertices[i] - centroid;
        const Vector2 center_to_vertice1 = vertices[(i + 1) % 4] - centroid;
        const f32 center_to_vertice_lenght2 = center_to_vertice.dot(center_to_vertice);
        const f32 center_to_vertice1_lenght2 = center_to_vertice1.dot(center_to_vertice1);
        const f32 inertia_triangle = mass_per_triangle_face * 
            (center_to_vertice_lenght2 + center_to_vertice1_lenght2
            + center_to_vertice.dot(center_to_vertice1)) / 6.f;
        
        inertia += inertia_triangle;
    } 

    return inertia;
}

Shape2D P2DShape::to_shape_2d() const
{
    return Shape2D
    {
        .vertices = 
        {
            vertices[0],
            vertices[1],
            vertices[2],
            vertices[3],
        }
    };
}

void P2DShape::_calc_aabb()
{
    aabb.min.x = math::min(
        vertices[0].x, vertices[1].x,
        vertices[2].x, vertices[3].x
    );

    aabb.min.y = math::min(
        vertices[0].y, vertices[1].y,
        vertices[2].y, vertices[3].y
    );

    aabb.max.x = math::max(
        vertices[0].x, vertices[1].x,
        vertices[2].x, vertices[3].x
    );

    aabb.max.y = math::max(
        vertices[0].y, vertices[1].y,
        vertices[2].y, vertices[3].y
    );
}

void P2DShape::_calc_normals()
{
    for (usize i = 0; i < 4; i++)
    {
        const Vector2& v1 = vertices[i];
        const Vector2& v2 = vertices[(i + 1) % 4];

        normals[i] = (v2 - v1).normalized().normal();
    }
}

void P2DShape::_calc_area()
{
    area = 0.f;

    // From: https://en.wikipedia.org/wiki/Polygon#Area
    for (usize i = 0; i < 4; i++)
    {
        const Vector2& v1 = vertices[i];
        const Vector2& v2 = vertices[(i + 1) % 4];
        area += v1.x * v2.y - v2.x * v1.y;
    }

    area /= 2.f;
}

void P2DShape::_calc_centroid()
{
    const f32 abs_area = area;
    centroid = Vector2();
    
    if (abs_area == 0.f)
    {
        return;
    }

    // From: https://en.wikipedia.org/wiki/Polygon#Centroid
    for (usize i = 0; i < 4; i++)
    {
        const Vector2& v1 = vertices[i];
        const Vector2& v2 = vertices[(i + 1) % 4];

        const f32 v1_v2_x = v1.x + v2.x;
        const f32 v1_v2_y = v1.y + v2.y;
        const f32 v1_v2 = v1.x * v2.y - v2.x * v1.y;

        centroid.x += v1_v2_x * v1_v2;
        centroid.y += v1_v2_y * v1_v2;
    }

    centroid /= (6.f * abs_area);
}

