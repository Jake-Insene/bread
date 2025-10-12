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
        Vector2 old_pos{};
        f32 old_rot = 0;

        PositionMode position_mode = POSITION_TOP_LEFT;
    } data;
    
    /*
    * When smoot_position is enabled camera_speed 
    * is used to update the camera current position
    */
    Vector2 camera_speed{1, 1};
    /*
    * Enable camera position interpolation.
    */
    bool smooth_position;

    void init(const CreateInfo& info);

    void enter();
    void exit();

    /*
    * Active/Deactivate this camera as the main scene camera.
    * 
    * @param enable True to set as main camera otherwise disable it.
    */
    void set_enable(bool enable) Function(FunctionNormal);

    /*
    * Set the position tracking mode.
    * 
    * If new_pm is POSITION_TOP_LEFT the window top left will correspond
    * to the camera transform position, for the other hand POSITION_CENTERED
    * 
    * @param new_pm The new Tracking Mode.
    */
    void set_position_mode(PositionMode new_pm) Function(FunctionNormal)
    { 
        data.position_mode = new_pm;
    }
    
    /*
    * @return The current position tracking mode.
    */
    [[nodiscard]] PositionMode get_position_mode() const Function(FunctionNormal)
    { 
        return data.position_mode;
    }
    
    [[nodiscard]] Transform2D get_camera_transform(bool update) Function(FunctionInternal);

};
