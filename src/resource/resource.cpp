#include "resource/resource.h"

#include "resource_manager.h"


void Resource::init(ResourceType rtype)
{
    type = rtype;
    path = String::with_allocator(ResourceManager::get_allocator());
}

void Resource::destroy()
{
    path.destroy();
}
