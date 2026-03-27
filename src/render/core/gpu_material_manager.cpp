#include "render/core/gpu_material_manager.h"


void GPUMaterialManager::initialize(const mem::Allocator& _allocator)
{
    allocator = _allocator;

    materials = FreeList<MaterialData, GPURenderMaterialID>::with_size(allocator, 4);
}

void GPUMaterialManager::shutdown()
{
    materials.destroy();
}
