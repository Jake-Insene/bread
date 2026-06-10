#include "renderer/render_pass.h"


void RenderPass::transition_to_render_attachment(GPU::CommandBufferID command_buffer, GPU::TextureID texture)
{
    GPU::PipelineTextureBarrier texture_barrier =
    {
        .src_masks = GPU::AccessMasks(),
        .dest_masks = GPU::AccessMasks::RenderAttachmentWrite,
        .src_layout = GPU::TextureLayout::Unknown,
        .dest_layout = GPU::TextureLayout::RenderAttachment,
        .texture = texture,
        .subresource_range = GPU::TextureSubresourceRange::color(0, 1, 0, 1),
    };
    GPU::command_buffer_pipeline_barrier(
        command_buffer,
        GPU::PipelineBarrier::texture_barrier(
            GPU::PipelineStages::RenderOutput, GPU::PipelineStages::RenderOutput,
            Slice(&texture_barrier, 1)
        )
    );
}

void RenderPass::transition_to_present(GPU::CommandBufferID command_buffer, GPU::TextureID texture)
{
    GPU::PipelineTextureBarrier texture_barrier =
    {
        .src_masks = GPU::AccessMasks::RenderAttachmentWrite,
        .dest_masks = GPU::AccessMasks(),
        .src_layout = GPU::TextureLayout::RenderAttachment,
        .dest_layout = GPU::TextureLayout::Present,
        .texture = texture,
        .subresource_range = GPU::TextureSubresourceRange::color(0, 1, 0, 1),
    };
    GPU::command_buffer_pipeline_barrier(
        command_buffer,
        GPU::PipelineBarrier::texture_barrier(
            GPU::PipelineStages::RenderOutput, GPU::PipelineStages::End,
            Slice(&texture_barrier, 1)
        )
    );
}

void RenderPass::begin(GPU::CommandBufferID command_buffer, const Vector2U& extent, const GPU::Viewport& viewport,
    const GPU::Scissor& scissor, GPU::TextureViewID render_attachment_view, const GPU::ClearValue& clear_value)
{
    GPU::AttachmentInfo render_attachment =
    {
        .texture_view = render_attachment_view,
        .layout = GPU::TextureLayout::RenderAttachment,
        .resolve_texture_view = GPU::TextureViewID::invalid(),
        .resolve_layout = GPU::TextureLayout::Unknown,
        .load_op = GPU::LoadOp::Clear,
        .store_op = GPU::StoreOp::Store,
        .clear_value = clear_value,
    };

    GPU::command_buffer_begin_renderpass(
        command_buffer,
        {
            .offset = Vector2I(0, 0),
            .extent = Vector3U(extent.x, extent.y, 1),
            .render_attachments = Slice(&render_attachment, 1),
            .depth_attachment = {},
            .stencil_attachment = {},
        }
    );

    GPU::command_buffer_set_viewports(command_buffer, 0, Slice(&viewport, 1));
    GPU::command_buffer_set_scissors(command_buffer, 0, Slice(&scissor, 1));
}

void RenderPass::end(GPU::CommandBufferID command_buffer)
{
    GPU::command_buffer_end_renderpass(command_buffer, {});
}
