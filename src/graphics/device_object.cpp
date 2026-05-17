#include "graphics/device_object.h"

#include "graphics/device.h"



namespace Graphics
{
    
void DeviceObject::init(mem::Allocator* _allocator, Device* _parent)
{
    parent = _parent;
    allocator = _allocator;
}

void DeviceObject::destroy()
{
    parent->release_object(this);
}

}
