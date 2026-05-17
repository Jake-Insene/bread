#pragma once
#include "mem/allocator.h"


namespace Graphics
{

struct Device;

struct DeviceObject
{
    Device* parent;
    mem::Allocator* allocator;

    void init(mem::Allocator* _allocator, Device* _parent);
    void destroy();
};

}
