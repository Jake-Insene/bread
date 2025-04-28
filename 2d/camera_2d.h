#pragma once
#include "2d/object_2d.h"
#include "math/projection.h"

struct Camera2D : Object2D
{
    OBJECT(Camera2D, Object2D);

    bool enable = false;

    void init(const CreateInfo& info);
    void start();

    void set_enable(bool _enable);
    
    void set_position(const Vector2& pos);
};
