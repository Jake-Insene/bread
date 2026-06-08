#include "graphics/command_buffer.h"

#include "graphics/command_pool.h"
#include "graphics/descriptor_set.h"
#include "graphics/pipeline_layout.h"
#include "graphics/pipeline.h"


namespace Graphics
{

void CommandBuffer::init(Mem::Allocator* _allocator, Device* _parent, CommandPool* command_pool)
{
    DeviceObject::init(_allocator, _parent);

    gpu_command_buffer = GPU::command_buffer_allocate(
        {
            .pool = command_pool->gpu_command_pool,
        }
    );
}

void CommandBuffer::destroy()
{
    GPU::command_buffer_free(gpu_command_buffer);
}

void CommandBuffer::begin()
{
    GPU::command_buffer_begin(gpu_command_buffer);
}

void CommandBuffer::end()
{
    GPU::command_buffer_end(gpu_command_buffer);
}

void CommandBuffer::begin_renderpass(const GPU::RenderPassBeginInfo& begin_info)
{
    GPU::command_buffer_begin_renderpass(gpu_command_buffer, begin_info);
}

void CommandBuffer::end_renderpass(const GPU::RenderPassEndInfo& end_info)
{
    GPU::command_buffer_end_renderpass(gpu_command_buffer, end_info);
}

void CommandBuffer::texture_barrier(const GPU::PipelineTextureBarrier& barrier)
{
    GPU::command_buffer_texture_barrier(gpu_command_buffer, barrier);
}

void CommandBuffer::bind_pipeline(GPU::PipelineBindPoint bind_point, GPU::PipelineID pipeline)
{
    GPU::command_buffer_bind_pipeline(gpu_command_buffer, bind_point, pipeline);
}

void CommandBuffer::bind_set(GPU::PipelineBindPoint bind_point, GPU::PipelineLayoutID pipeline_layout, u32 base_set, const Slice<GPU::DescriptorSetID>& sets)
{
    GPU::command_buffer_bind_descriptor_sets(gpu_command_buffer, bind_point, pipeline_layout, base_set, sets);
}

void CommandBuffer::bind_vertex_buffers(u32 base_binding, const Slice<GPU::BufferID>& buffers, const Slice<usize>& offsets)
{
    GPU::command_buffer_bind_vertex_buffers(gpu_command_buffer, base_binding, buffers, offsets);
}

void CommandBuffer::set_viewports(u32 base_viewport, const Slice<const GPU::Viewport>& viewports)
{
    GPU::command_buffer_set_viewports(gpu_command_buffer, base_viewport, viewports);
}

void CommandBuffer::set_scissors(u32 base_scissor, const Slice<const GPU::Scissor>& scissors)
{
    GPU::command_buffer_set_scissors(gpu_command_buffer, base_scissor, scissors);
}

void CommandBuffer::draw(u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance)
{
    GPU::command_buffer_draw(gpu_command_buffer, vertex_count, instance_count, base_vertex, base_instance);
}

}
