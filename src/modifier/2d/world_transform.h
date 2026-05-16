#pragma once
#include "math/transform_2d.h"
#include "modifier/core/modifier.h"




struct [[nodiscard]] WorldTransform : Modifier
{
    Transform2D transform;

    void init();
    void deinit();
};

