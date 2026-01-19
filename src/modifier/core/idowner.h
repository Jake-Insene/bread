#pragma once
#include "modifier/modifier.h"
#include "object/object_id.h"



struct [[nodiscard]] IDOwner : Modifier
{
    ObjectID id;
};

