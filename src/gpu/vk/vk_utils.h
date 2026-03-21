#pragma once
#include "gpu/gpu.h"
#include "gpu/vk/vk_header.h"


struct VkUtils
{
    static VkCompareOp _vk_get_compare_op(GPU::CompareOp compare_op);
    static VkPresentModeKHR _vk_get_present_mode(GPU::PresentMode present_mode);
    static VkMemoryPropertyFlags _vk_get_memory_properties(GPU::HeapUsage heap_usage);
    static VkBufferUsageFlags _vk_get_buffer_usage(GPU::BufferUsage buffer_usage);
    static VkFilter _vk_get_filter(GPU::Filter filter);
    static VkSamplerMipmapMode _vk_get_mipmap_mode(GPU::SamplerMipMapMode mipmap_mode);
    static VkSamplerAddressMode _vk_get_address_mode(GPU::SamplerAddressMode address_mode);
    static VkImageType _vk_get_image_type(GPU::TextureType texture_type);
    static VkImageViewType _vk_get_image_view_type(GPU::TextureType texture_type);
    static VkFormat _vk_get_texture_format(GPU::TextureFormat texture_format);
    static VkImageTiling _vk_get_tiling(GPU::TextureTiling texture_tiling);
    static VkImageUsageFlags _vk_get_texture_usage(GPU::TextureUsage texture_usage);
    static VkDescriptorType _vk_get_descriptor_type(GPU::DescriptorType descriptor_type);
    static VkPipelineStageFlags _vk_get_pipeline_stages(GPU::PipelineStages stages);
    static VkImageAspectFlags _vk_get_aspect_masks(GPU::TextureAspect aspects);
    static VkAccessFlags _vk_get_access_masks(GPU::AccessMasks access_masks);
    static VkImageLayout _vk_get_image_layout(GPU::TextureLayout texture_layout);
    static VkShaderStageFlags _vk_get_shader_stage(GPU::ShaderStage shader_stage);
    static VkVertexInputRate _vk_get_input_rate(GPU::InputRate input_rate);
    static VkFormat _vk_get_vertex_format(GPU::VertexFormat vertex_format);
    static VkPrimitiveTopology _vk_get_topology(GPU::PrimitiveTopology primitive_topology);
    static VkPolygonMode _vk_get_polygon_mode(GPU::PolygonMode polygon_mode);
    static VkCullModeFlags _vk_get_cull_mode(GPU::CullMode cull_mode);
    static VkFrontFace _vk_get_front_face(GPU::FrontFace front_face);
    static VkSampleCountFlagBits _vk_get_samples(GPU::SampleCount sample_count);
    static VkPipelineBindPoint _vk_get_bind_point(GPU::PipelineBindPoint bind_point);
};
