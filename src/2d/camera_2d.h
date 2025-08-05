#pragma once
#include "2d/object_2d.h"
#include "math/projection.h"


struct Camera2D : Object2D
{
    OBJECT(Camera2D, Object2D);

    enum PositionMode
    {
        POSITION_CENTERED,
    };

    // As everything in a struct is public we need to hide data
    // that should not be modified/access directly, this also
    // resolve some namespace problems.
    struct InternalData
    {
        bool enable = false;
        Vector2 old_pos = Vector2(0);
        f32 old_rot = 0;

        PositionMode position_mode = POSITION_CENTERED;
    } data;
    
    f32 speed = 1;

    void init(const CreateInfo& info);

    void enter();
    void exit();

    void set_enable(bool _enable);
    void set_position_mode(PositionMode new_pm) { data.position_mode = new_pm; }
    [[nodiscard]] PositionMode get_position_mode() const { return data.position_mode; }
    
    Transform2D get_camera_transform();

};
