#include "render/renderer/renderer.h"


void Renderer::initialize(const SystemInitializeInfo& info)
{
    allocator = info.allocator;
}

void Renderer::shutdown() {}
