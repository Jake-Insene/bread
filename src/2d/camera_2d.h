#pragma once
#include "2d/object_2d.h"
#include "math/projection.h"


struct Camera2D : Object2D
{
    OBJECT(Camera2D, Object2D);

    enum PositionMode
    {
        POSITION_TOP_LEFT,
        POSITION_CENTERED,
    };

    struct InternalData
    {
        bool enable = false;
        Vector2 old_pos = Vector2(0);
        f32 old_rot = 0;

        PositionMode position_mode = POSITION_TOP_LEFT;
    } data;
    
    Vector2 camera_speed = Vector2(1, 1);
    bool smooth_position;

    void init(const CreateInfo& info);

    void enter();
    void exit();

    void set_enable(bool _enable);
    void set_position_mode(PositionMode new_pm) { data.position_mode = new_pm; }
    [[nodiscard]] PositionMode get_position_mode() const { return data.position_mode; }
    
    Transform2D get_camera_transform();

};
