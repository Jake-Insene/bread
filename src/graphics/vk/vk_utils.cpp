#include "graphics/vk/vk_utils.h"


VkCompareOp VkUtils::_vk_get_compare_op(Graphics::CompareOp compare_op)
{
    switch(compare_op)
    {
    case Graphics::CompareOp::Never:
        return VK_COMPARE_OP_NEVER;
    case Graphics::CompareOp::Always:
        return VK_COMPARE_OP_ALWAYS;
    case Graphics::CompareOp::Equal:
        return VK_COMPARE_OP_EQUAL;
    case Graphics::CompareOp::NotEqual:
        return VK_COMPARE_OP_NOT_EQUAL;
    case Graphics::CompareOp::Less:
        return VK_COMPARE_OP_LESS;
    case Graphics::CompareOp::LessOrEqual:
        return VK_COMPARE_OP_LESS_OR_EQUAL;
    case Graphics::CompareOp::Greater:
        return VK_COMPARE_OP_GREATER;
    case Graphics::CompareOp::GreaterOrEqual:
        return VK_COMPARE_OP_GREATER_OR_EQUAL;
    default:
        break;
    }

    
    VKFailOn(true, "invalid compare op");
    return VkCompareOp(0);
}

VkPresentModeKHR VkUtils::_vk_get_present_mode(Graphics::PresentMode present_mode)
{
    switch(present_mode)
    {
    case Graphics::PresentMode::Immediate:
        return VK_PRESENT_MODE_IMMEDIATE_KHR;
        case Graphics::PresentMode::VSync:
        return VK_PRESENT_MODE_FIFO_KHR;
    default:
        break;
    }

    VKFailOn(true, "invalid present mode");
    return VkPresentModeKHR(0);
}

VkMemoryPropertyFlags VkUtils::_vk_get_memory_properties(Graphics::HeapUsage heap_usage)
{
    switch(heap_usage)
    {
    case Graphics::HeapUsage::CPUExclusive:
        return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    case Graphics::HeapUsage::GPUExclusive:
        return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    case Graphics::HeapUsage::CPUGPUCoherent:
        return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    default:
        break;
    }

    VKFailOn(true, "invalid heap usage");
    return VkMemoryPropertyFlags(0);
}

VkBufferUsageFlags VkUtils::_vk_get_buffer_usage(Graphics::BufferUsage buffer_usage)
{
    VkBufferUsageFlags vk_flags = 0;

    if(HasValue(buffer_usage & Graphics::BufferUsage::VertexBuffer))
    {
        vk_flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if(HasValue(buffer_usage & Graphics::BufferUsage::IndexBuffer))
    {
        vk_flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if(HasValue(buffer_usage & Graphics::BufferUsage::UniformBuffer))
    {
        vk_flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if(HasValue(buffer_usage & Graphics::BufferUsage::TransferSource))
    {
        vk_flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if(HasValue(buffer_usage & Graphics::BufferUsage::TransferDestination))
    {
        vk_flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }
    
    return vk_flags;
}

VkFilter VkUtils::_vk_get_filter(Graphics::Filter filter)
{
    switch(filter)
    {
    case Graphics::Filter::Nearest:
        return VK_FILTER_NEAREST;
    case Graphics::Filter::Linear:
        return VK_FILTER_LINEAR;
    default:
        break;
    }

    VKFailOn(true, "invalid filter type");
    return VkFilter(0);
}

VkSamplerMipmapMode VkUtils::_vk_get_mipmap_mode(Graphics::SamplerMipMapMode mipmap_mode)
{
    switch(mipmap_mode)
    {
    case Graphics::SamplerMipMapMode::Nearest:
        return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    case Graphics::SamplerMipMapMode::Linear:
        return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    default:
        break;
    }

    VKFailOn(true, "invalid mip map mode");
    return VkSamplerMipmapMode(0);
}

VkSamplerAddressMode VkUtils::_vk_get_address_mode(Graphics::SamplerAddressMode address_mode)
{
    switch(address_mode)
    {
    case Graphics::SamplerAddressMode::Repeat:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case Graphics::SamplerAddressMode::MirroredRepeat:
        return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case Graphics::SamplerAddressMode::ClampToEdge:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case Graphics::SamplerAddressMode::ClampToBorder:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    default:
        break;
    }

    VKFailOn(true, "invalid sampler address mode");
    return VkSamplerAddressMode(0);
}

VkImageType VkUtils::_vk_get_image_type(Graphics::TextureType texture_type)
{
    switch(texture_type)
    {
    case Graphics::TextureType::Texture2D:
        return VK_IMAGE_TYPE_2D;
    default:
        break;
    }

    VKFailOn(true, "invalid texture type");
    return VkImageType(0);
}

VkImageViewType VkUtils::_vk_get_image_view_type(Graphics::TextureType texture_type)
{
    switch(texture_type)
    {
    case Graphics::TextureType::Texture2D:
        return VK_IMAGE_VIEW_TYPE_2D;
    default:
        break;
    }

    VKFailOn(true, "invalid texture type");
    return VkImageViewType(0);
}

VkFormat VkUtils::_vk_get_texture_format(Graphics::TextureFormat texture_format)
{
    switch(texture_format)
    {
    case Graphics::TextureFormat::RGBA8Srgb:
        return VK_FORMAT_R8G8B8A8_SRGB;
    case Graphics::TextureFormat::RGB8Srgb:
        return VK_FORMAT_R8G8B8_SRGB;
    case Graphics::TextureFormat::R8Srgb:
        return VK_FORMAT_R8_SRGB;
    default:
        break;
    }

    VKFailOn(true, "invalid texture format");
    return VK_FORMAT_UNDEFINED;
}

VkImageTiling VkUtils::_vk_get_tiling(Graphics::TextureTiling texture_tiling)
{
    switch(texture_tiling)
    {
    case Graphics::TextureTiling::Optimal:
        return VK_IMAGE_TILING_OPTIMAL;
    case Graphics::TextureTiling::Linear:
        return VK_IMAGE_TILING_LINEAR;
    default:
        break;
    }

    VKFailOn(true, "invalid texture tiling");
    return VkImageTiling(0);
}

VkImageUsageFlags VkUtils::_vk_get_texture_usage(Graphics::TextureUsage texture_usage)
{
    VkImageUsageFlags flags = 0;
    if(HasValue(texture_usage & Graphics::TextureUsage::TransferSource))
    {
        flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if(HasValue(texture_usage & Graphics::TextureUsage::TransferDestination))
    {
        flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if(HasValue(texture_usage & Graphics::TextureUsage::Sampled))
    {
        flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if(HasValue(texture_usage & Graphics::TextureUsage::Storage))
    {
        flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if(HasValue(texture_usage & Graphics::TextureUsage::RenderOutput))
    {
        flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    return flags;
}

VkDescriptorType VkUtils::_vk_get_descriptor_type(Graphics::DescriptorType descriptor_type)
{
    switch(descriptor_type)
    {
    case Graphics::DescriptorType::UniformBuffer:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case Graphics::DescriptorType::StorageBuffer:
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case Graphics::DescriptorType::CombinedTextureSampler:
        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    default:
        break;
    }

    VKFailOn(true, "invalid descriptor type");
    return VkDescriptorType(0);
}

VkPipelineStageFlags VkUtils::_vk_get_pipeline_stages(Graphics::PipelineStages stages)
{
    VkPipelineStageFlags vk_flags = 0;

    if(HasValue(stages & Graphics::PipelineStages::Begin))
    {
        vk_flags |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    if(HasValue(stages & Graphics::PipelineStages::VertexInput))
    {
        vk_flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    }
    if (HasValue(stages & Graphics::PipelineStages::VertexShader))
    {
        vk_flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }
    if (HasValue(stages & Graphics::PipelineStages::FragmentShader))
    {
        vk_flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    if (HasValue(stages & Graphics::PipelineStages::EarlyFragmentTestShader))
    {
        vk_flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    if (HasValue(stages & Graphics::PipelineStages::LateFragmentTestShader))
    {
        vk_flags |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    }
    if (HasValue(stages & Graphics::PipelineStages::ComputeShader))
    {
        vk_flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    if(HasValue(stages & Graphics::PipelineStages::RenderOutput))
    {
        vk_flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    if (HasValue(stages & Graphics::PipelineStages::Transfer))
    {
        vk_flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    if(HasValue(stages & Graphics::PipelineStages::End))
    {
        vk_flags |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }

    return vk_flags;
}

VkImageAspectFlags VkUtils::_vk_get_aspect_masks(Graphics::TextureAspect aspects)
{
    VkImageAspectFlags vk_aspects = 0;

    if (HasValue(aspects & Graphics::TextureAspect::Color))
    {
        vk_aspects |= VK_IMAGE_ASPECT_COLOR_BIT;
    }
    if (HasValue(aspects & Graphics::TextureAspect::Depth))
    {
        vk_aspects |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    if (HasValue(aspects & Graphics::TextureAspect::Stencil))
    {
        vk_aspects |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    return vk_aspects;
}

VkAccessFlags VkUtils::_vk_get_access_masks(Graphics::AccessMasks access_masks)
{
    VkAccessFlags vk_access_masks = 0;

    if (HasValue(access_masks & Graphics::AccessMasks::RenderOutputRead))
    {
        vk_access_masks |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    }
    if (HasValue(access_masks & Graphics::AccessMasks::RenderOutputWrite))
    {
        vk_access_masks |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }
    if (HasValue(access_masks & Graphics::AccessMasks::TransferRead))
    {
        vk_access_masks |= VK_ACCESS_TRANSFER_READ_BIT;
    }
    if (HasValue(access_masks & Graphics::AccessMasks::TransferWrite))
    {
        vk_access_masks |= VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    if (HasValue(access_masks & Graphics::AccessMasks::ShaderRead))
    {
        vk_access_masks |= VK_ACCESS_SHADER_READ_BIT;
    }
    if (HasValue(access_masks & Graphics::AccessMasks::ShaderWrite))
    {
        vk_access_masks |= VK_ACCESS_SHADER_WRITE_BIT;
    }

    return vk_access_masks;
}

VkImageLayout VkUtils::_vk_get_image_layout(Graphics::TextureLayout texture_layout)
{
    switch (texture_layout)
    {
    case Graphics::TextureLayout::Unknown:
        return VK_IMAGE_LAYOUT_UNDEFINED;
    case Graphics::TextureLayout::RenderOutput:
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case Graphics::TextureLayout::Present:
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    case Graphics::TextureLayout::ShaderReadOnly:
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case Graphics::TextureLayout::TransferSource:
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case Graphics::TextureLayout::TransferDestination:
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    default:
        break;
    }

    VKFailOn(true, "invalid texture layout");
    return VK_IMAGE_LAYOUT_UNDEFINED;
}

VkShaderStageFlags VkUtils::_vk_get_shader_stage(Graphics::ShaderStage shader_stage)
{
    VkShaderStageFlags vk_flags = 0;
    
    if(HasValue(shader_stage & Graphics::ShaderStage::Vertex))
    {
        vk_flags |= VK_SHADER_STAGE_VERTEX_BIT;
    }

    if(HasValue(shader_stage & Graphics::ShaderStage::Fragment))
    {
        vk_flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    return vk_flags;
}

VkVertexInputRate VkUtils::_vk_get_input_rate(Graphics::InputRate input_rate)
{
    switch(input_rate)
    {
    case Graphics::InputRate::Vertex:
        return VK_VERTEX_INPUT_RATE_VERTEX;
    case Graphics::InputRate::Instance:
        return VK_VERTEX_INPUT_RATE_INSTANCE;
    default:
        break;
    };

    VKFailOn(true, "invalid input rate");
    return VkVertexInputRate();
}

VkFormat VkUtils::_vk_get_vertex_format(Graphics::VertexFormat vertex_format)
{
    switch(vertex_format)
    {
    case Graphics::VertexFormat::RGBA32Float:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case Graphics::VertexFormat::RGB32Float:
        return VK_FORMAT_R32G32B32_SFLOAT;
    case Graphics::VertexFormat::RG32Float:
        return VK_FORMAT_R32G32_SFLOAT;
    case Graphics::VertexFormat::R32Float:
        return VK_FORMAT_R32_SFLOAT;
    default:
        break;
    };

    VKFailOn(true, "invalid vertex format");
    return VkFormat();
}

VkPrimitiveTopology VkUtils::_vk_get_topology(Graphics::PrimitiveTopology primitive_topology)
{
    switch(primitive_topology)
    {
    case Graphics::PrimitiveTopology::TriangleList:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case Graphics::PrimitiveTopology::LineList:
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    default:
        break;
    };

    VKFailOn(true, "invalid primitive topology");
    return VkPrimitiveTopology();
}

VkPolygonMode VkUtils::_vk_get_polygon_mode(Graphics::PolygonMode polygon_mode)
{
    switch(polygon_mode)
    {
    case Graphics::PolygonMode::Fill:
        return VK_POLYGON_MODE_FILL;
    case Graphics::PolygonMode::Line:
        return VK_POLYGON_MODE_LINE;
    case Graphics::PolygonMode::Point:
        return VK_POLYGON_MODE_POINT;
    default:
        break;
    };

    VKFailOn(true, "invalid polygon mode");
    return VkPolygonMode();
}

VkCullModeFlags VkUtils::_vk_get_cull_mode(Graphics::CullMode cull_mode)
{
    switch(cull_mode)
    {
    case Graphics::CullMode::Front:
        return VK_CULL_MODE_FRONT_BIT;
    case Graphics::CullMode::Back:
        return VK_CULL_MODE_BACK_BIT;
    case Graphics::CullMode::FrontAndBack:
        return VK_CULL_MODE_FRONT_AND_BACK;
    default:
        break;
    };

    VKFailOn(true, "invalid cull mode");
    return VkCullModeFlags();
}

VkFrontFace VkUtils::_vk_get_front_face(Graphics::FrontFace front_face)
{
    // Important: The vulkan backend flips the Y to allow positive up,
    // that changes the winding order so it needs to be flipped.
    switch(front_face)
    {
    case Graphics::FrontFace::CounterClockWise:
        return VK_FRONT_FACE_CLOCKWISE;
    case Graphics::FrontFace::ClockWise:
        return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    default:
        break;
    };

    VKFailOn(true, "invalid front face");
    return VkFrontFace();
}

VkSampleCountFlagBits VkUtils::_vk_get_samples(Graphics::SampleCount sample_count)
{
    switch(sample_count)
    {
    case Graphics::SampleCount::Sample1:
        return VK_SAMPLE_COUNT_1_BIT;
    case Graphics::SampleCount::Sample2:
        return VK_SAMPLE_COUNT_2_BIT;
    case Graphics::SampleCount::Sample4:
        return VK_SAMPLE_COUNT_4_BIT;
    case Graphics::SampleCount::Sample8:
        return VK_SAMPLE_COUNT_8_BIT;
    case Graphics::SampleCount::Sample16:
        return VK_SAMPLE_COUNT_16_BIT;
    case Graphics::SampleCount::Sample32:
        return VK_SAMPLE_COUNT_32_BIT;
    default:
        break;
    }
    
    VKFailOn(true, "invalid sample count");
    return VkSampleCountFlagBits();
}

VkPipelineBindPoint VkUtils::_vk_get_bind_point(Graphics::PipelineBindPoint bind_point)
{
    switch(bind_point)
    {
    case Graphics::PipelineBindPoint::Graphics:
        return VK_PIPELINE_BIND_POINT_GRAPHICS;
    default:
        break;
    }

    VKFailOn(true, "invalid bind point");
    return VkPipelineBindPoint();
}