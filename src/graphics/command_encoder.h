#pragma once
#include "collections/array.h"
#include "gpu/gpu.h"
#include "mem/allocator.h"


struct RenderDevice;

namespace Graphics
{

struct Pipeline;
struct Buffer;
struct DescriptorSet;

struct CommandEncoder
{
    mem::Allocator allocator;
    GPU::CommandBufferID command_buffer;

    void begin();
    void end();

    void begin_renderpass(const GPU::RenderPassBeginInfo& begin_info);
    void end_renderpass(const GPU::RenderPassEndInfo& end_info);

    void texture_barrier(const GPU::PipelineTextureBarrier& barrier);

    void bind_pipeline(GPU::PipelineBindPoint bind_point, Pipeline* pipeline);
    void bind_set(GPU::PipelineBindPoint bind_point, Pipeline* pipeline, u32 base_set, const Slice<DescriptorSet*>& sets);
    void bind_vertex_buffers(u32 base_binding, const Slice<Buffer*>& buffers, const Slice<usize>& offsets);

    void set_viewports(u32 base_viewport, const Slice<GPU::Viewport>& viewports);
    void set_scissors(u32 base_scissor, const Slice<GPU::Scissor>& scissors);

    void draw(u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance);
};

}
