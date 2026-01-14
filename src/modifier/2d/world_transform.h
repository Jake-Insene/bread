#pragma once
#include "modifier/modifier.h"
#include "math/transform_2d.h"




struct [[nodiscard]] WorldTransform : Modifier
{
    Transform2D transform;
};

