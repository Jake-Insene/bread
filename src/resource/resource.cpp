#include "resource/resource.h"

#include "resource/resource_manager.h"
#include "engine/engine.h"


Result<Resource*, Error> Resource::_load_resource(ResourceType type, ResourceTypeSpecification spec, StringView path)
{
    return Engine::get_resource_manager()->load_resource(type, spec, path);
}

Resource::Resource(const ResourceCreateInfo& info)
: allocator(info.allocator), type(info.resource_type), path(allocator, 0, {})
{
    allocator = info.allocator;
    type = info.resource_type;
}

Resource::~Resource()
{}
