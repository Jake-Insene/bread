#pragma once
#include "2d/object_2d.h"

#include "debug/debug.h"
#include "physics/physics_2d.h"


struct Body2D;

struct Area2D : Object2D
{
    OBJECT(Area2D, Object2D);

    static void _bind_vtable(Area2D::VTable& vtable);

    using CollisionMask = Physics2D::CollisionMask;

    struct InternalData
    {
        Physics2D::AreaID area_id = Physics2D::AreaID::InvalidID;

        CollisionMask residence_mask = Physics2D::DEFAULT_COLLISION_MASK;
    } data;

    Event<void(Object::*)(Body2D*)> on_body_enter;
    Event<void(Object::*)(Body2D*)> on_body_exit;

    void init(const CreateInfo&);
    void deinit();

    void enter();
    void exit();

    void transform_changed();

    void set_shape(const Shape2D& shape);
    Shape2D get_shape();

    void set_residence_mask(CollisionMask mask);
    [[nodiscard]] CollisionMask get_residence_mask() const { return data.residence_mask; }

    static void _on_body_enter(void* _this, Object2D* obj);
    static void _on_body_exit(void* _this, Object2D* obj);
};
