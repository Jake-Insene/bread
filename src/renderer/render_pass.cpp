#include "renderer/render_pass.h"


void RenderPass::transition_to_render_attachment(Graphics::CommandEncoder* encoder, GPU::TextureID texture)
{
    encoder->texture_barrier(
        {
            .src_stages = GPU::PipelineStages::RenderOutput,
            .dest_stages = GPU::PipelineStages::RenderOutput,
            .src_masks = GPU::AccessMasks(0),
            .dest_masks = GPU::AccessMasks::RenderAttachmentWrite,
            .src_layout = GPU::TextureLayout::Unknown,
            .dest_layout = GPU::TextureLayout::RenderAttachment,
            .texture = texture,
            .subresource_range = GPU::TextureSubresourceRange::color(0, 1, 0, 1),
        }
    );
}

void RenderPass::transition_to_present(Graphics::CommandEncoder* encoder, GPU::TextureID texture)
{
    encoder->texture_barrier(
        {
            .src_stages = GPU::PipelineStages::RenderOutput,
            .dest_stages = GPU::PipelineStages::End,
            .src_masks = GPU::AccessMasks::RenderAttachmentWrite,
            .dest_masks = GPU::AccessMasks(0),
            .src_layout = GPU::TextureLayout::RenderAttachment,
            .dest_layout = GPU::TextureLayout::Present,
            .texture = texture,
            .subresource_range = GPU::TextureSubresourceRange::color(0, 1, 0, 1),
        }
    );
}

void RenderPass::begin(Graphics::CommandEncoder* encoder, const Vector2U& extent, const GPU::Viewport& viewport,
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

    encoder->begin_renderpass(
        {
            .offset = Vector2I(0, 0),
            .extent = Vector3U(extent.x, extent.y, 1),
            .render_attachments = Slice(&render_attachment, 1),
            .depth_attachment = {},
            .stencil_attachment = {},
        }
    );

    encoder->set_viewports(0, Slice(&viewport, 1));
    encoder->set_scissors(0, Slice(&scissor, 1));
}

void RenderPass::end(Graphics::CommandEncoder* encoder)
{
    encoder->end_renderpass({});
}
