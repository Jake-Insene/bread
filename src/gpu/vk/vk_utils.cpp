#include "gpu/vk/vk_utils.h"


VkCompareOp VkUtils::_vk_get_compare_op(GPU::CompareOp compare_op)
{
    switch(compare_op)
    {
    case GPU::CompareOp::Never:
        return VK_COMPARE_OP_NEVER;
    case GPU::CompareOp::Always:
        return VK_COMPARE_OP_ALWAYS;
    case GPU::CompareOp::Equal:
        return VK_COMPARE_OP_EQUAL;
    case GPU::CompareOp::NotEqual:
        return VK_COMPARE_OP_NOT_EQUAL;
    case GPU::CompareOp::Less:
        return VK_COMPARE_OP_LESS;
    case GPU::CompareOp::LessOrEqual:
        return VK_COMPARE_OP_LESS_OR_EQUAL;
    case GPU::CompareOp::Greater:
        return VK_COMPARE_OP_GREATER;
    case GPU::CompareOp::GreaterOrEqual:
        return VK_COMPARE_OP_GREATER_OR_EQUAL;
    default:
        break;
    }

    
    VKFailOn(true, "invalid compare op");
    return VkCompareOp(0);
}

VkPresentModeKHR VkUtils::_vk_get_present_mode(GPU::PresentMode present_mode)
{
    switch(present_mode)
    {
    case GPU::PresentMode::Immediate:
        return VK_PRESENT_MODE_IMMEDIATE_KHR;
        case GPU::PresentMode::VSync:
        return VK_PRESENT_MODE_FIFO_KHR;
    default:
        break;
    }

    VKFailOn(true, "invalid present mode");
    return VkPresentModeKHR(0);
}

VkMemoryPropertyFlags VkUtils::_vk_get_memory_properties(GPU::HeapUsage heap_usage)
{
    switch(heap_usage)
    {
    case GPU::HeapUsage::CPUExclusive:
        return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    case GPU::HeapUsage::GPUExclusive:
        return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    case GPU::HeapUsage::CPUGPUCoherent:
        return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    default:
        break;
    }

    VKFailOn(true, "invalid heap usage");
    return VkMemoryPropertyFlags(0);
}

VkBufferUsageFlags VkUtils::_vk_get_buffer_usage(GPU::BufferUsage buffer_usage)
{
    VkBufferUsageFlags vk_flags = 0;

    if(HasValue(buffer_usage & GPU::BufferUsage::VertexBuffer))
    {
        vk_flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if(HasValue(buffer_usage & GPU::BufferUsage::IndexBuffer))
    {
        vk_flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if(HasValue(buffer_usage & GPU::BufferUsage::UniformBuffer))
    {
        vk_flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if(HasValue(buffer_usage & GPU::BufferUsage::TransferSource))
    {
        vk_flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if(HasValue(buffer_usage & GPU::BufferUsage::TransferDestination))
    {
        vk_flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }
    
    return vk_flags;
}

VkFilter VkUtils::_vk_get_filter(GPU::Filter filter)
{
    switch(filter)
    {
    case GPU::Filter::Nearest:
        return VK_FILTER_NEAREST;
    case GPU::Filter::Linear:
        return VK_FILTER_LINEAR;
    default:
        break;
    }

    VKFailOn(true, "invalid filter type");
    return VkFilter(0);
}

VkSamplerMipmapMode VkUtils::_vk_get_mipmap_mode(GPU::SamplerMipMapMode mipmap_mode)
{
    switch(mipmap_mode)
    {
    case GPU::SamplerMipMapMode::Nearest:
        return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    case GPU::SamplerMipMapMode::Linear:
        return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    default:
        break;
    }

    VKFailOn(true, "invalid mip map mode");
    return VkSamplerMipmapMode(0);
}

VkSamplerAddressMode VkUtils::_vk_get_address_mode(GPU::SamplerAddressMode address_mode)
{
    switch(address_mode)
    {
    case GPU::SamplerAddressMode::Repeat:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case GPU::SamplerAddressMode::MirroredRepeat:
        return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case GPU::SamplerAddressMode::ClampToEdge:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case GPU::SamplerAddressMode::ClampToBorder:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    default:
        break;
    }

    VKFailOn(true, "invalid sampler address mode");
    return VkSamplerAddressMode(0);
}

VkImageType VkUtils::_vk_get_image_type(GPU::TextureType texture_type)
{
    switch(texture_type)
    {
    case GPU::TextureType::Texture2D:
        return VK_IMAGE_TYPE_2D;
    default:
        break;
    }

    VKFailOn(true, "invalid texture type");
    return VkImageType(0);
}

VkImageViewType VkUtils::_vk_get_image_view_type(GPU::TextureType texture_type)
{
    switch(texture_type)
    {
    case GPU::TextureType::Texture2D:
        return VK_IMAGE_VIEW_TYPE_2D;
    default:
        break;
    }

    VKFailOn(true, "invalid texture type");
    return VkImageViewType(0);
}

VkFormat VkUtils::_vk_get_texture_format(GPU::TextureFormat texture_format)
{
    switch(texture_format)
    {
    case GPU::TextureFormat::RGBA8Srgb:
        return VK_FORMAT_R8G8B8A8_SRGB;
    case GPU::TextureFormat::RGB8Srgb:
        return VK_FORMAT_R8G8B8_SRGB;
    case GPU::TextureFormat::R8Srgb:
        return VK_FORMAT_R8_SRGB;
    default:
        break;
    }

    VKFailOn(true, "invalid texture format");
    return VK_FORMAT_UNDEFINED;
}

VkImageTiling VkUtils::_vk_get_tiling(GPU::TextureTiling texture_tiling)
{
    switch(texture_tiling)
    {
    case GPU::TextureTiling::Optimal:
        return VK_IMAGE_TILING_OPTIMAL;
    case GPU::TextureTiling::Linear:
        return VK_IMAGE_TILING_LINEAR;
    default:
        break;
    }

    VKFailOn(true, "invalid texture tiling");
    return VkImageTiling(0);
}

VkImageUsageFlags VkUtils::_vk_get_texture_usage(GPU::TextureUsage texture_usage)
{
    VkImageUsageFlags flags = 0;
    if(HasValue(texture_usage & GPU::TextureUsage::TransferSource))
    {
        flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if(HasValue(texture_usage & GPU::TextureUsage::TransferDestination))
    {
        flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if(HasValue(texture_usage & GPU::TextureUsage::Sampled))
    {
        flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if(HasValue(texture_usage & GPU::TextureUsage::Storage))
    {
        flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if(HasValue(texture_usage & GPU::TextureUsage::RenderOutput))
    {
        flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    return flags;
}

VkDescriptorType VkUtils::_vk_get_descriptor_type(GPU::DescriptorType descriptor_type)
{
    switch(descriptor_type)
    {
    case GPU::DescriptorType::UniformBuffer:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case GPU::DescriptorType::StorageBuffer:
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case GPU::DescriptorType::CombinedTextureSampler:
        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    default:
        break;
    }

    VKFailOn(true, "invalid descriptor type");
    return VkDescriptorType(0);
}

VkPipelineStageFlags VkUtils::_vk_get_pipeline_stages(GPU::PipelineStages stages)
{
    VkPipelineStageFlags vk_flags = 0;

    if(HasValue(stages & GPU::PipelineStages::Begin))
    {
        vk_flags |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    if(HasValue(stages & GPU::PipelineStages::VertexInput))
    {
        vk_flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    }
    if (HasValue(stages & GPU::PipelineStages::VertexShader))
    {
        vk_flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }
    if (HasValue(stages & GPU::PipelineStages::FragmentShader))
    {
        vk_flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    if (HasValue(stages & GPU::PipelineStages::EarlyFragmentTestShader))
    {
        vk_flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    if (HasValue(stages & GPU::PipelineStages::LateFragmentTestShader))
    {
        vk_flags |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    }
    if (HasValue(stages & GPU::PipelineStages::ComputeShader))
    {
        vk_flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    if(HasValue(stages & GPU::PipelineStages::RenderOutput))
    {
        vk_flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    if (HasValue(stages & GPU::PipelineStages::Transfer))
    {
        vk_flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    if(HasValue(stages & GPU::PipelineStages::End))
    {
        vk_flags |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }

    return vk_flags;
}

VkImageAspectFlags VkUtils::_vk_get_aspect_masks(GPU::TextureAspect aspects)
{
    VkImageAspectFlags vk_aspects = 0;

    if (HasValue(aspects & GPU::TextureAspect::Color))
    {
        vk_aspects |= VK_IMAGE_ASPECT_COLOR_BIT;
    }
    if (HasValue(aspects & GPU::TextureAspect::Depth))
    {
        vk_aspects |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    if (HasValue(aspects & GPU::TextureAspect::Stencil))
    {
        vk_aspects |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    return vk_aspects;
}

VkAccessFlags VkUtils::_vk_get_access_masks(GPU::AccessMasks access_masks)
{
    VkAccessFlags vk_access_masks = 0;

    if (HasValue(access_masks & GPU::AccessMasks::RenderOutputRead))
    {
        vk_access_masks |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    }
    if (HasValue(access_masks & GPU::AccessMasks::RenderOutputWrite))
    {
        vk_access_masks |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }
    if (HasValue(access_masks & GPU::AccessMasks::TransferRead))
    {
        vk_access_masks |= VK_ACCESS_TRANSFER_READ_BIT;
    }
    if (HasValue(access_masks & GPU::AccessMasks::TransferWrite))
    {
        vk_access_masks |= VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    if (HasValue(access_masks & GPU::AccessMasks::ShaderRead))
    {
        vk_access_masks |= VK_ACCESS_SHADER_READ_BIT;
    }
    if (HasValue(access_masks & GPU::AccessMasks::ShaderWrite))
    {
        vk_access_masks |= VK_ACCESS_SHADER_WRITE_BIT;
    }

    return vk_access_masks;
}

VkImageLayout VkUtils::_vk_get_image_layout(GPU::TextureLayout texture_layout)
{
    switch (texture_layout)
    {
    case GPU::TextureLayout::Unknown:
        return VK_IMAGE_LAYOUT_UNDEFINED;
    case GPU::TextureLayout::RenderOutput:
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case GPU::TextureLayout::Present:
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    case GPU::TextureLayout::ShaderReadOnly:
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case GPU::TextureLayout::TransferSource:
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case GPU::TextureLayout::TransferDestination:
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    default:
        break;
    }

    VKFailOn(true, "invalid texture layout");
    return VK_IMAGE_LAYOUT_UNDEFINED;
}

VkShaderStageFlags VkUtils::_vk_get_shader_stage(GPU::ShaderStage shader_stage)
{
    VkShaderStageFlags vk_flags = 0;
    
    if(HasValue(shader_stage & GPU::ShaderStage::Vertex))
    {
        vk_flags |= VK_SHADER_STAGE_VERTEX_BIT;
    }

    if(HasValue(shader_stage & GPU::ShaderStage::Fragment))
    {
        vk_flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    return vk_flags;
}

VkVertexInputRate VkUtils::_vk_get_input_rate(GPU::InputRate input_rate)
{
    switch(input_rate)
    {
    case GPU::InputRate::Vertex:
        return VK_VERTEX_INPUT_RATE_VERTEX;
    case GPU::InputRate::Instance:
        return VK_VERTEX_INPUT_RATE_INSTANCE;
    default:
        break;
    };

    VKFailOn(true, "invalid input rate");
    return VkVertexInputRate();
}

VkFormat VkUtils::_vk_get_vertex_format(GPU::VertexFormat vertex_format)
{
    switch(vertex_format)
    {
    case GPU::VertexFormat::RGBA32Float:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case GPU::VertexFormat::RGB32Float:
        return VK_FORMAT_R32G32B32_SFLOAT;
    case GPU::VertexFormat::RG32Float:
        return VK_FORMAT_R32G32_SFLOAT;
    case GPU::VertexFormat::R32Float:
        return VK_FORMAT_R32_SFLOAT;
    default:
        break;
    };

    VKFailOn(true, "invalid vertex format");
    return VkFormat();
}

VkPrimitiveTopology VkUtils::_vk_get_topology(GPU::PrimitiveTopology primitive_topology)
{
    switch(primitive_topology)
    {
    case GPU::PrimitiveTopology::TriangleList:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case GPU::PrimitiveTopology::LineList:
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    default:
        break;
    };

    VKFailOn(true, "invalid primitive topology");
    return VkPrimitiveTopology();
}

VkPolygonMode VkUtils::_vk_get_polygon_mode(GPU::PolygonMode polygon_mode)
{
    switch(polygon_mode)
    {
    case GPU::PolygonMode::Fill:
        return VK_POLYGON_MODE_FILL;
    case GPU::PolygonMode::Line:
        return VK_POLYGON_MODE_LINE;
    case GPU::PolygonMode::Point:
        return VK_POLYGON_MODE_POINT;
    default:
        break;
    };

    VKFailOn(true, "invalid polygon mode");
    return VkPolygonMode();
}

VkCullModeFlags VkUtils::_vk_get_cull_mode(GPU::CullMode cull_mode)
{
    switch(cull_mode)
    {
    case GPU::CullMode::Front:
        return VK_CULL_MODE_FRONT_BIT;
    case GPU::CullMode::Back:
        return VK_CULL_MODE_BACK_BIT;
    case GPU::CullMode::FrontAndBack:
        return VK_CULL_MODE_FRONT_AND_BACK;
    default:
        break;
    };

    VKFailOn(true, "invalid cull mode");
    return VkCullModeFlags();
}

VkFrontFace VkUtils::_vk_get_front_face(GPU::FrontFace front_face)
{
    // Important: The vulkan backend flips the Y to allow positive up,
    // that changes the winding order so it needs to be flipped.
    switch(front_face)
    {
    case GPU::FrontFace::CounterClockWise:
        return VK_FRONT_FACE_CLOCKWISE;
    case GPU::FrontFace::ClockWise:
        return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    default:
        break;
    };

    VKFailOn(true, "invalid front face");
    return VkFrontFace();
}

VkSampleCountFlagBits VkUtils::_vk_get_samples(GPU::SampleCount sample_count)
{
    switch(sample_count)
    {
    case GPU::SampleCount::Sample1:
        return VK_SAMPLE_COUNT_1_BIT;
    case GPU::SampleCount::Sample2:
        return VK_SAMPLE_COUNT_2_BIT;
    case GPU::SampleCount::Sample4:
        return VK_SAMPLE_COUNT_4_BIT;
    case GPU::SampleCount::Sample8:
        return VK_SAMPLE_COUNT_8_BIT;
    case GPU::SampleCount::Sample16:
        return VK_SAMPLE_COUNT_16_BIT;
    case GPU::SampleCount::Sample32:
        return VK_SAMPLE_COUNT_32_BIT;
    default:
        break;
    }
    
    VKFailOn(true, "invalid sample count");
    return VkSampleCountFlagBits();
}

VkPipelineBindPoint VkUtils::_vk_get_bind_point(GPU::PipelineBindPoint bind_point)
{
    switch(bind_point)
    {
    case GPU::PipelineBindPoint::Graphics:
        return VK_PIPELINE_BIND_POINT_GRAPHICS;
    default:
        break;
    }

    VKFailOn(true, "invalid bind point");
    return VkPipelineBindPoint();
}