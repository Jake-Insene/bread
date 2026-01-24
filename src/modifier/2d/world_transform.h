#pragma once
#include "math/transform_2d.h"
#include "modifier/modifier.h"




struct [[nodiscard]] WorldTransform : Modifier
{
    Transform2D transform;

    void init();
    void deinit();
};

