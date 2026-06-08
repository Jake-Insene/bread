#pragma once
#include "gpu/gpu.h"
#include "graphics/device_object.h"
#include "mem/allocator.h"


struct RenderDevice;

namespace Graphics
{

struct CommandBuffer : DeviceObject
{
    GPU::CommandBufferID gpu_command_buffer;

    void init(Mem::Allocator* _allocator, Device* _parent, CommandPool* command_pool);
    void destroy();

    void begin();
    void end();

    void begin_renderpass(const GPU::RenderPassBeginInfo& begin_info);
    void end_renderpass(const GPU::RenderPassEndInfo& end_info);

    void texture_barrier(const GPU::PipelineTextureBarrier& barrier);

    void bind_pipeline(GPU::PipelineBindPoint bind_point, GPU::PipelineID pipeline);
    void bind_set(GPU::PipelineBindPoint bind_point, GPU::PipelineLayoutID pipeline_layout, u32 base_set, const Slice<GPU::DescriptorSetID>& sets);
    void bind_vertex_buffers(u32 base_binding, const Slice<GPU::BufferID>& buffers, const Slice<usize>& offsets);

    void set_viewports(u32 base_viewport, const Slice<const GPU::Viewport>& viewports);
    void set_scissors(u32 base_scissor, const Slice<const GPU::Scissor>& scissors);

    void draw(u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance);
};

}
