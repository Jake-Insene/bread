#include "resource/resource.h"

#include "resource/resource_manager.h"


Result<Resource*, Error> Resource::_load_resource(ResourceType type, ResourceTypeSpecification spec, StringView path)
{
    return ResourceManager::load_resource(type, spec, path);
}



void Resource::init(ResourceType resource_type)
{
    type = resource_type;
    path = String::with_allocator(ResourceManager::get_allocator());
}

void Resource::destroy()
{
    path.destroy();
}
