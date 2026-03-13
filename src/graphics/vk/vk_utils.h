#pragma once
#include "graphics/graphics.h"
#include "graphics/vk/vk_header.h"


struct VkUtils
{
    static VkCompareOp _vk_get_compare_op(Graphics::CompareOp compare_op);
    static VkPresentModeKHR _vk_get_present_mode(Graphics::PresentMode present_mode);
    static VkMemoryPropertyFlags _vk_get_memory_properties(Graphics::HeapUsage heap_usage);
    static VkBufferUsageFlags _vk_get_buffer_usage(Graphics::BufferUsage buffer_usage);
    static VkFilter _vk_get_filter(Graphics::Filter filter);
    static VkSamplerMipmapMode _vk_get_mipmap_mode(Graphics::SamplerMipMapMode mipmap_mode);
    static VkSamplerAddressMode _vk_get_address_mode(Graphics::SamplerAddressMode address_mode);
    static VkImageType _vk_get_image_type(Graphics::TextureType texture_type);
    static VkImageViewType _vk_get_image_view_type(Graphics::TextureType texture_type);
    static VkFormat _vk_get_texture_format(Graphics::TextureFormat texture_format);
    static VkImageTiling _vk_get_tiling(Graphics::TextureTiling texture_tiling);
    static VkImageUsageFlags _vk_get_texture_usage(Graphics::TextureUsage texture_usage);
    static VkDescriptorType _vk_get_descriptor_type(Graphics::DescriptorType descriptor_type);
    static VkPipelineStageFlags _vk_get_pipeline_stages(Graphics::PipelineStages stages);
    static VkImageAspectFlags _vk_get_aspect_masks(Graphics::TextureAspect aspects);
    static VkAccessFlags _vk_get_access_masks(Graphics::AccessMasks access_masks);
    static VkImageLayout _vk_get_image_layout(Graphics::TextureLayout texture_layout);
    static VkShaderStageFlags _vk_get_shader_stage(Graphics::ShaderStage shader_stage);
    static VkVertexInputRate _vk_get_input_rate(Graphics::InputRate input_rate);
    static VkFormat _vk_get_vertex_format(Graphics::VertexFormat vertex_format);
    static VkPrimitiveTopology _vk_get_topology(Graphics::PrimitiveTopology primitive_topology);
    static VkPolygonMode _vk_get_polygon_mode(Graphics::PolygonMode polygon_mode);
    static VkCullModeFlags _vk_get_cull_mode(Graphics::CullMode cull_mode);
    static VkFrontFace _vk_get_front_face(Graphics::FrontFace front_face);
    static VkSampleCountFlagBits _vk_get_samples(Graphics::SampleCount sample_count);
    static VkPipelineBindPoint _vk_get_bind_point(Graphics::PipelineBindPoint bind_point);
};
