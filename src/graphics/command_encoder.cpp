#include "graphics/command_encoder.h"

#include "graphics/descriptor_set.h"
#include "graphics/pipeline_layout.h"
#include "graphics/pipeline.h"


namespace Graphics
{

void CommandEncoder::begin()
{
    GPU::command_buffer_begin(command_buffer);
}

void CommandEncoder::end()
{
    GPU::command_buffer_end(command_buffer);
}

void CommandEncoder::begin_renderpass(const GPU::RenderPassBeginInfo& begin_info)
{
    GPU::command_buffer_begin_renderpass(command_buffer, begin_info);
}

void CommandEncoder::end_renderpass(const GPU::RenderPassEndInfo& end_info)
{
    GPU::command_buffer_end_renderpass(command_buffer, end_info);
}

void CommandEncoder::texture_barrier(const GPU::PipelineTextureBarrier& barrier)
{
    GPU::command_buffer_texture_barrier(command_buffer, barrier);
}

void CommandEncoder::bind_pipeline(GPU::PipelineBindPoint bind_point, GPU::PipelineID pipeline)
{
    GPU::command_buffer_bind_pipeline(command_buffer, bind_point, pipeline);
}

void CommandEncoder::bind_set(GPU::PipelineBindPoint bind_point, GPU::PipelineLayoutID pipeline_layout, u32 base_set, const Slice<GPU::DescriptorSetID>& sets)
{
    GPU::command_buffer_bind_descriptor_sets(command_buffer, bind_point, pipeline_layout, base_set, sets);
}

void CommandEncoder::bind_vertex_buffers(u32 base_binding, const Slice<GPU::BufferID>& buffers, const Slice<usize>& offsets)
{
    GPU::command_buffer_bind_vertex_buffers(command_buffer, base_binding, buffers, offsets);
}

void CommandEncoder::set_viewports(u32 base_viewport, const Slice<const GPU::Viewport>& viewports)
{
    GPU::command_buffer_set_viewports(command_buffer, base_viewport, viewports);
}

void CommandEncoder::set_scissors(u32 base_scissor, const Slice<const GPU::Scissor>& scissors)
{
    GPU::command_buffer_set_scissors(command_buffer, base_scissor, scissors);
}

void CommandEncoder::draw(u32 vertex_count, u32 instance_count, u32 base_vertex, u32 base_instance)
{
    GPU::command_buffer_draw(command_buffer, vertex_count, instance_count, base_vertex, base_instance);
}

}
