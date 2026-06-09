#include "graphics/command_buffer.h"

#include "graphics/command_pool.h"
#include "graphics/descriptor_set.h"
#include "graphics/pipeline_layout.h"
#include "graphics/pipeline.h"
#include "os/os.h"


namespace Graphics
{

void CommandBuffer::init(Mem::Allocator* _allocator, Device* _parent, CommandPool* command_pool)
{
    DeviceObject::init(_allocator, _parent);

    ConstructObject(tmp_allocator);
    tmp_allocator.init(OS::map_memory(DefaultStackSize, OS::MapAccess::MapReadWrite));

    gpu_command_buffer = GPU::command_buffer_allocate(
        {
            .pool = command_pool->gpu_command_pool,
        }
    );
}

void CommandBuffer::destroy()
{
    OS::unmap_memory(tmp_allocator.sp);

    GPU::command_buffer_free(gpu_command_buffer);
}

void CommandBuffer::begin()
{
    tmp_allocator.reset();
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

void CommandBuffer::pipeline_barrier(const GPU::PipelineBarrier& pipeline_barrier)
{
    GPU::command_buffer_pipeline_barrier(gpu_command_buffer, pipeline_barrier);
}

void CommandBuffer::copy_buffer_to_texture(const GPU::CopyBufferToTextureInfo& copy_info)
{
    GPU::command_buffer_copy_buffer_to_texture(
        gpu_command_buffer, copy_info);
}

void CommandBuffer::copy_buffer(const GPU::BufferCopyInfo& copy_info)
{
    GPU::command_buffer_copy_buffer(
        gpu_command_buffer, copy_info);
}

void CommandBuffer::bind_pipeline(GPU::PipelineBindPoint bind_point, const Pipeline* pipeline)
{
    GPU::command_buffer_bind_pipeline(gpu_command_buffer, bind_point, pipeline->gpu_pipeline);
}

void CommandBuffer::bind_set(GPU::PipelineBindPoint bind_point, const PipelineLayout* pipeline_layout, u32 base_set, const Slice<const DescriptorSet*>& sets)
{
    Slice<GPU::DescriptorSetID> gpu_sets = tmp_allocator.array<GPU::DescriptorSetID>(sets.len);
    for(usize i = 0; i < gpu_sets.len; i++)
    {
        gpu_sets[i] = sets[i]->gpu_descriptor_set;
    }

    GPU::command_buffer_bind_descriptor_sets(gpu_command_buffer, bind_point, pipeline_layout->gpu_pipeline_layout, base_set, gpu_sets);
}

void CommandBuffer::bind_vertex_buffers(u32 base_binding, const Slice<const Buffer*>& buffers, const Slice<usize>& offsets)
{
    Slice<GPU::BufferID> gpu_buffers = tmp_allocator.array<GPU::BufferID>(buffers.len);
    for(usize i = 0; i < gpu_buffers.len; i++)
    {
        gpu_buffers[i] = buffers[i]->gpu_buffer;
    }

    GPU::command_buffer_bind_vertex_buffers(gpu_command_buffer, base_binding, gpu_buffers, offsets);
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
