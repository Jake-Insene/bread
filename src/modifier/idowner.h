#pragma once
#include "modifier/core/modifier.h"
#include "object/object_id.h"



struct [[nodiscard]] IDOwner : Modifier
{
    ObjectID id;
};

