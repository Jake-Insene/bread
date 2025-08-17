#include "resource/resource.h"

#include "resource_manager.h"


void Resource::init(ResourceType resource_type)
{
    type = resource_type;
    path = String::with_allocator(ResourceManager::get_allocator());
}

void Resource::destroy()
{
    path.destroy();
}
