#pragma once
#include "2d/object_2d.h"

#include "debug/debug.h"
#include "physics/physics_2d.h"


struct Body2D;

struct Area2D : Object2D
{
    OBJECT(Area2D, Object2D);

    using CollisionMask = Physics2D::CollisionMask;

    // As everything in a struct is public we need to hide data
    // that should not be modified/access directly, this also
    // resolve some namespace problems.
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

    void add_shape(const Shape2D& new_shape);
    void remove_shape(usize index);
    usize get_shape_count();
    void set_shape(usize index, const Shape2D& shape);
    Shape2D get_shape(usize index);

    void set_residence_mask(CollisionMask mask);
    [[nodiscard]] CollisionMask get_residence_mask() const { return data.residence_mask; }

    static void _on_body_enter(void* _this, Object2D* obj);
    static void _on_body_exit(void* _this, Object2D* obj);
};
