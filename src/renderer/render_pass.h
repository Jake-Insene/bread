#pragma once
#include "gpu/gpu.h"



struct RenderPass
{
    GPU::AttachmentInfo render_attachments[GPU::MaxRenderAttachmentCount];

    GPU::RenderPassBeginInfo begin_info;

    static void transition_to_render_attachment(GPU::CommandBufferID command_buffer, GPU::TextureID texture);
    static void transition_to_present(GPU::CommandBufferID command_buffer, GPU::TextureID texture);

    static void begin(GPU::CommandBufferID command_buffer, const Vector2U& extent, const GPU::Viewport& viewport,
        const GPU::Scissor& scissor, GPU::TextureViewID render_attachment_view, const GPU::ClearValue& clear_value);

    static void end(GPU::CommandBufferID command_buffer);
};

