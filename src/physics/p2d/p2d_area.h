#pragma once
#include "collections/array.h"

#include "physics/p2d/p2d_shape.h"
#include "physics/p2d/p2d_types.h"



struct [[nodiscard]] P2DArea
{
    struct BodyInArea
    {
        bool is_inside;
        i64 check_counter;
    };

    struct InternalData
    {
        P2DShape shape_transformed;
        Transform2D transform;
    } data;

    P2DShape shape;
    Opaque* user_data;
    Physics2D::AreaID self;
    Physics2D::CollisionMask residence_mask;

    Physics2D::EventOnBodyEnter on_body_enter;
    Physics2D::EventOnBodyExit on_body_exit;

    bool is_active;

    HashMap<Physics2D::BodyID, BodyInArea> bodies_inside;
    Array<PhysicsTileCoord> tiles_on;
    i64 check_counter;

    void init(const mem::Allocator& allocator, Physics2D::AreaID id, Opaque* ud);
    void destroy();

    void set_shape_from_2d(const Shape2D& new_shape);
    P2DShape get_shape_transformed() const { return data.shape_transformed; }

    void set_transform(const Transform2D& new_transform);
    Transform2D get_transform() const { return data.transform; }
};