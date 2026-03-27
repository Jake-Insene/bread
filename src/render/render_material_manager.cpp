#include "render/render_material_manager.h"


void RenderMaterialManager::initialize(const mem::Allocator& _allocator)
{
    allocator = _allocator;

    materials = FreeList<MaterialData, RenderMaterialID>::with_size(allocator, 4);
}

void RenderMaterialManager::shutdown()
{
    materials.destroy();
}
