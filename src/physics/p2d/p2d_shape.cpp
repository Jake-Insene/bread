#include "physics/p2d/p2d_shape.h"
#include "p2d_shape.h"



void P2DShape::init(Mem::Allocator* allocator)
{
    vertices = Array<Vector2>::with_size(allocator, 4);
    normals = Array<Vector2>::with_size(allocator, 4);

    area = 0.F;
    centroid = Vector2();
    aabb = AABB();
}

void P2DShape::destroy()
{
    vertices.destroy();
    normals.destroy();
}

P2DShape P2DShape::copy() const
{
    if(vertices.count == 0)
    {
        return P2DShape();
    }

    return P2DShape
    {
        .vertices = vertices.copy(vertices.allocator),
        .normals = normals.copy(normals.allocator),
        .area = area,
        .centroid = centroid,
        .aabb = aabb,
    };
}

void P2DShape::set_from_shape_2d(const Shape2D& shape)
{
    vertices.resize(shape.vertices.count);
    for(usize i = 0; i < shape.vertices.count; i++)
    {
        vertices.get(i) = shape.vertices.get(i);
    }

    normals.resize(vertices.count);

    _calc_aabb();
    _calc_normals();
    _calc_area();
    _calc_centroid();
}

void P2DShape::set_from_shape(const P2DShape& other_shape)
{
    vertices.resize(other_shape.vertices.count);
    normals.resize(other_shape.vertices.count);
    area = other_shape.area;
    centroid = other_shape.centroid;
    aabb = other_shape.aabb;

    for(usize i = 0; i < other_shape.vertices.count; i++)
    {
        vertices.get(i) = other_shape.vertices.get(i);
        normals.get(i) = other_shape.normals.get(i);
    }
}


void P2DShape::apply_transform(const Transform2D& transform)
{
    if(vertices.count == 0)
    {
        return;
    }

    (void)vertices.iter().transform([&](const Vector2& v){ return transform * v; });

    centroid = transform * centroid;

    // The transformation could rotate the vertices
    _calc_aabb();
    _calc_normals();
}

void P2DShape::translate(const Vector2& translation)
{
    if(vertices.count == 0)
    {
        return;
    }

    (void)vertices.iter().transform([&](const Vector2& v){ return v + translation; });

    centroid += translation;
 
    _calc_aabb();
}

void P2DShape::rotate(const f32 r)
{
    if(vertices.count == 0)
    {
        return;
    }

    for (usize i = 0; i < vertices.count; i++)
    {
        vertices.get(i) = Vector2::rotate_around_point(vertices.get(i), centroid, r);
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
    f32 inertia = 0.F;
    const f32 mass_per_triangle_face = mass / 4.F;
    for(usize i = 0; i < vertices.count; i++)
    {
        const Vector2 center_to_vertice = vertices.get(i) - centroid;
        const Vector2 center_to_vertice1 = vertices.get((i + 1) % vertices.count) - centroid;
        const f32 center_to_vertice_length2 = center_to_vertice.dot(center_to_vertice);
        const f32 center_to_vertice1_length2 = center_to_vertice1.dot(center_to_vertice1);
        const f32 inertia_triangle = mass_per_triangle_face * 
            (center_to_vertice_length2 + center_to_vertice1_length2
            + center_to_vertice.dot(center_to_vertice1)) / 6.F;
        
        inertia += inertia_triangle;
    } 

    return inertia;
}

Shape2D P2DShape::to_shape_2d() const
{
    return Shape2D
    {
        .vertices = vertices.copy(vertices.allocator),
    };
}

void P2DShape::_calc_aabb()
{
    if(vertices.count == 0)
    {
        return;
    }

    aabb.min.x = vertices.get(0).x;
    aabb.min.y = vertices.get(0).y;
    aabb.max.x = vertices.get(0).x;
    aabb.max.y = vertices.get(0).y;

    for(const Vector2& v : vertices.iter())
    {
        aabb.min.x = Math::min(aabb.min.x, v.x);
        aabb.min.y = Math::min(aabb.min.y, v.y);

        aabb.max.x = Math::max(aabb.max.x, v.x);
        aabb.max.y = Math::max(aabb.max.y, v.y);
    }
}

void P2DShape::_calc_normals()
{
    if(vertices.count == 0)
    {
        return;
    }

    for (usize i = 0; i < vertices.count; i++)
    {
        const Vector2& v1 = vertices.get(i);
        const Vector2& v2 = vertices.get((i + 1) % vertices.count);

        normals.get(i) = (v2 - v1).normalized().normal();
    }
}

void P2DShape::_calc_area()
{
    if(vertices.count == 0)
    {
        return;
    }

    area = 0.F;

    // From: https://en.wikipedia.org/wiki/Polygon#Area
    for (usize i = 0; i < vertices.count; i++)
    {
        const Vector2& v1 = vertices.get(i);
        const Vector2& v2 = vertices.get((i + 1) % vertices.count);
        area += (v1.x * v2.y) - (v2.x * v1.y);
    }

    area /= 2.F;
}

void P2DShape::_calc_centroid()
{
    if(vertices.count == 0)
    {
        return;
    }

    // Use signed area to keep the centroid calculation correct
    centroid = Vector2();
    
    if (area == 0.F)
    {
        return;
    }

    // From: https://en.wikipedia.org/wiki/Polygon#Centroid
    for (usize i = 0; i < vertices.count; i++)
    {
        const Vector2& v1 = vertices.get(i);
        const Vector2& v2 = vertices.get((i + 1) % vertices.count);

        const f32 v1_v2_x = v1.x + v2.x;
        const f32 v1_v2_y = v1.y + v2.y;

        const f32 v1_v2 = (v1.x * v2.y) - (v2.x * v1.y);

        centroid.x += v1_v2_x * v1_v2;
        centroid.y += v1_v2_y * v1_v2;
    }

    centroid /= (6.F * area);
}

