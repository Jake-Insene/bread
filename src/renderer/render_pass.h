#pragma once
#include "gpu/gpu.h"
#include "graphics/device_object.h"



struct RenderPass
{
    GPU::AttachmentInfo render_attachments[GPU::MaxRenderAttachmentCount];

    GPU::RenderPassBeginInfo begin_info;

    static void transition_to_render_attachment(Graphics::CommandBuffer* command_buffer, GPU::TextureID texture);
    static void transition_to_present(Graphics::CommandBuffer* command_buffer, GPU::TextureID texture);

    static void begin(Graphics::CommandBuffer* command_buffer, const Vector2U& extent, const GPU::Viewport& viewport,
        const GPU::Scissor& scissor, GPU::TextureViewID render_attachment_view, const GPU::ClearValue& clear_value);

    static void end(Graphics::CommandBuffer* command_buffer);
};


struct RenderPassRenderAttachmentList
{
    GPU::AttachmentInfo render_attachments[GPU::MaxRenderAttachmentCount];
};

struct RenderPassS
{
};
