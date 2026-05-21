#pragma once
#include "mem/allocator.h"


namespace Graphics
{

struct Device;

struct DeviceObject
{
    Device* parent;
    Mem::Allocator* allocator;

    void init(Mem::Allocator* _allocator, Device* _parent);
    void destroy();
};

}
