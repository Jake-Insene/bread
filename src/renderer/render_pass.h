#pragma once
#include "graphics/command_encoder.h"



struct RenderPass
{
    GPU::AttachmentInfo render_attachments[GPU::MaxRenderAttachmentCount];

    GPU::RenderPassBeginInfo begin_info;

    static void transition_to_render_attachment(Graphics::CommandEncoder* encoder, GPU::TextureID texture);
    static void transition_to_present(Graphics::CommandEncoder* encoder, GPU::TextureID texture);

    static void begin(Graphics::CommandEncoder* encoder, const Vector2U& extent, const GPU::Viewport& viewport,
        const GPU::Scissor& scissor, GPU::TextureViewID render_attachment_view, const GPU::ClearValue& clear_value);

    static void end(Graphics::CommandEncoder* encoder);
};


struct RenderPassRenderAttachmentList
{
    GPU::AttachmentInfo render_attachments[GPU::MaxRenderAttachmentCount];
};

struct RenderPassS
{
};
