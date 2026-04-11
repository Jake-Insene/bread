#include "renderer/renderer_2d.h"



void Renderer2D::init(const Renderer2DCreateInfo& info)
{
    allocator = info.allocator;

    buffers.init(
        {
            .allocator = allocator,
            .graphics_device = info.graphics_device,
            .gpu_memory_allocator = info.gpu_memory_allocator,
            .buffer_size = sizeof(BaseInstance) * MaxInstancePerFramedBuffer,
            .frame_count = 3,
            .usage = GPU::BufferUsage::VertexBuffer,
        }
    );
}

void Renderer2D::destroy()
{
    buffers.destroy();   
}
