#pragma once
#include "gpu/gpu.h"
#include "graphics/device_object.h"
#include "mem/stack_allocator.h"


namespace Graphics
{

struct CommandBuffer : DeviceObject
{
    static constexpr usize DefaultStackSize = 4096;

    Mem::StackAllocator tmp_allocator;
    GPU::CommandBufferID gpu_command_buffer;

    void init(Mem::Allocator* _allocator, Device* _parent, CommandPool* command_pool);
    void destroy();

    void begin();
    void end();

    void begin_renderpass(const GPU::RenderPassBeginInfo& begin_info);
    void end_renderpass(const GPU::RenderPassEndInfo& end_info);

    void pipeline_barrier(const GPU::PipelineBarrier& pipeline_barrier);

    void copy_buffer_to_texture(const GPU::CopyBufferToTextureInfo& copy_info);
	void copy_buffer(const GPU::CopyBufferInfo& copy_info);

    void bind_pipeline(GPU::PipelineBindPoint bind_point, const Pipeline* pipeline);
    void bind_set(GPU::PipelineBindPoint bind_point, const PipelineLayout* pipeline_layout, u32 base_set, const Slice<const DescriptorSet*>& sets);
    void bind_vertex_buffers(u32 base_binding, const Slice<const Buffer*>& buffers, const Slice<usize>& offsets);

    void set_viewports(u32 base_viewport, const Slice<const GPU::Viewport>& viewports);
    void set_scissors(u32 base_scissor, const Slice<const GPU::Scissor>& scissors);

    void draw(u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance);
};

}
