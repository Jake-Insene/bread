#include "gpu/vk/vk_utils.h"


static constexpr VkAttachmentLoadOp VkLoadOps[] =
{
    VkAttachmentLoadOp(), // Unknown
    VK_ATTACHMENT_LOAD_OP_LOAD, // Load
    VK_ATTACHMENT_LOAD_OP_CLEAR, // Clear
    VK_ATTACHMENT_LOAD_OP_DONT_CARE, // DontCare
};

static constexpr VkAttachmentStoreOp VkStoreOps[] =
{
    VkAttachmentStoreOp(), // Unknown
    VK_ATTACHMENT_STORE_OP_STORE, // Store
    VK_ATTACHMENT_STORE_OP_DONT_CARE, // DontCare
};

static constexpr VkCompareOp VkCompareOps[] =
{
    VkCompareOp(), // Unknown
    VK_COMPARE_OP_NEVER, // Never
    VK_COMPARE_OP_ALWAYS, // Always
    VK_COMPARE_OP_EQUAL, // Equal
    VK_COMPARE_OP_NOT_EQUAL, // NotEqual
    VK_COMPARE_OP_LESS, // Less
    VK_COMPARE_OP_LESS_OR_EQUAL, // LessOrEqual
    VK_COMPARE_OP_GREATER, // Greater
    VK_COMPARE_OP_GREATER_OR_EQUAL, // GreaterOrEqual
};

static constexpr VkPresentModeKHR VkPresentModes[] =
{
    VkPresentModeKHR(), // Unknown
    VK_PRESENT_MODE_IMMEDIATE_KHR, // Immediate
    VK_PRESENT_MODE_FIFO_KHR, // VSync
};

static constexpr VkMemoryPropertyFlags VkMemoryProperties[] =
{
    VkMemoryPropertyFlags(), // Unknown
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // CPUExclusive
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, // GPUExclusive
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // CPUGPUCoherent
};

static constexpr VkFilter VkFilters[] =
{
    VkFilter(), // Unknown
    VK_FILTER_NEAREST, // Nearest
    VK_FILTER_LINEAR, // Linear
};

static constexpr VkSamplerMipmapMode VkMipMapModes[] =
{
    VkSamplerMipmapMode(), // Unknown
    VK_SAMPLER_MIPMAP_MODE_NEAREST, // Nearest
    VK_SAMPLER_MIPMAP_MODE_LINEAR, // Linear
};

static constexpr VkSamplerAddressMode VkAddressModes[] =
{
    VkSamplerAddressMode(), // Unknown
    VK_SAMPLER_ADDRESS_MODE_REPEAT,// Repeat
    VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,// MirroredRepeat
    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,// ClampToEdge
    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,// ClampToBorder
};

static constexpr VkImageType VkImageTypes[] =
{
    VkImageType(), // Unknown
    VK_IMAGE_TYPE_2D,// Texture2D
};

static constexpr VkImageViewType VkImageViewTypes[] =
{
    VkImageViewType(), // Unknown
    VK_IMAGE_VIEW_TYPE_2D,// Texture2D
};

static constexpr VkComponentSwizzle VkComponentSwizzles[] =
{
    VkComponentSwizzle(), // Unknown
	VK_COMPONENT_SWIZZLE_IDENTITY, // Identity
	VK_COMPONENT_SWIZZLE_ZERO, // Zero
	VK_COMPONENT_SWIZZLE_ONE, // One
	VK_COMPONENT_SWIZZLE_R, // Red
	VK_COMPONENT_SWIZZLE_G, // Green
	VK_COMPONENT_SWIZZLE_B, // Blue
	VK_COMPONENT_SWIZZLE_A, // Alpha
};

static constexpr VkFormat VkTextureFormats[] =
{
    VkFormat(), // Unknown
	VK_FORMAT_R8G8B8A8_SRGB, // RGBA8Srgb
	VK_FORMAT_R8G8B8_SRGB, // RGB8Srgb
	VK_FORMAT_R8G8_SRGB, // RG8Srgb
	VK_FORMAT_R8_SRGB, // R8Srgb
	VK_FORMAT_R8G8B8A8_UNORM, // RGBA8Unorm
	VK_FORMAT_R8G8B8_UNORM, // RGB8Unorm
	VK_FORMAT_R8G8_UNORM, // RG8Unorm
	VK_FORMAT_R8_UNORM, // R8Unorm
	VK_FORMAT_B8G8R8A8_SRGB, // BGRA8Srgb
	VK_FORMAT_B8G8R8A8_UNORM, // BGRA8Unorm
};

static constexpr VkImageTiling VkImageTilings[] =
{
    VkImageTiling(), // Unknown
    VK_IMAGE_TILING_OPTIMAL, // Optimal
	VK_IMAGE_TILING_LINEAR, // Linear
};

static constexpr VkDescriptorType VkDescriptorTypes[] =
{
    VkDescriptorType(), // Unknown
    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // UniformBuffer
	VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, // StorageBuffer
	VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, // CombinedTextureSampler
};

static constexpr VkImageLayout VkImageLayouts[] =
{
    VkImageLayout(), // Unknown
	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // RenderAttachment
	VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, // Present
	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, // ShaderReadOnly
	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, // TransferSource
	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // TransferDestination
};

static constexpr VkVertexInputRate VkVertexInputRates[] =
{
    VkVertexInputRate(), // Unknown
	VK_VERTEX_INPUT_RATE_VERTEX, // Vertex
	VK_VERTEX_INPUT_RATE_INSTANCE, // Instance
};

static constexpr VkFormat VkVertexFormats[] =
{
    VkFormat(), // Unknown
	VK_FORMAT_R32G32B32A32_SFLOAT, // RGBA32Float
	VK_FORMAT_R32G32B32_SFLOAT, // RGB32Float
	VK_FORMAT_R32G32_SFLOAT, // RG32Float
	VK_FORMAT_R32_SFLOAT, // R32Float
};

static constexpr VkPrimitiveTopology VkPrimitiveTopologies[] =
{
    VkPrimitiveTopology(), // Unknown
	VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, // TriangleList
	VK_PRIMITIVE_TOPOLOGY_LINE_LIST, // LineList
};

static constexpr VkPolygonMode VkPolygonModes[] =
{
    VkPolygonMode(), // Unknown
	VK_POLYGON_MODE_FILL, // Fill
	VK_POLYGON_MODE_LINE, // Line
	VK_POLYGON_MODE_POINT, // Point
};

static constexpr VkCullModeFlags VkCullModes[] =
{
    VkCullModeFlags(), // Unknown
	VK_CULL_MODE_FRONT_BIT, // Front
	VK_CULL_MODE_BACK_BIT, // Back
	VK_CULL_MODE_FRONT_AND_BACK, // FrontAndBack
};

// Important: The vulkan backend flips the Y to allow positive up,
// that changes the winding order so it needs to be flipped.
static constexpr VkFrontFace VkFrontFaces[] =
{
    VkFrontFace(), // Unknown
	VK_FRONT_FACE_CLOCKWISE, // CounterClockWise
	VK_FRONT_FACE_COUNTER_CLOCKWISE, // ClockWise
};

static constexpr VkSampleCountFlagBits VkSampleCounts[] =
{
    VkSampleCountFlagBits(), // Unknown
	VK_SAMPLE_COUNT_1_BIT, // Sample1
	VK_SAMPLE_COUNT_2_BIT, // Sample2
	VK_SAMPLE_COUNT_4_BIT, // Sample4
	VK_SAMPLE_COUNT_8_BIT, // Sample8
	VK_SAMPLE_COUNT_16_BIT, // Sample16
	VK_SAMPLE_COUNT_32_BIT, // Sample32
	VK_SAMPLE_COUNT_64_BIT, // Sample64
};

static constexpr VkLogicOp VkLogicOps[] =
{
    VkLogicOp(), // Unknown
	VK_LOGIC_OP_CLEAR, // Clear
    VK_LOGIC_OP_AND, // And
    VK_LOGIC_OP_AND_REVERSE, // AndReverse
    VK_LOGIC_OP_COPY, // Copy
    VK_LOGIC_OP_AND_INVERTED, // AndInverted
    VK_LOGIC_OP_NO_OP, // NoOp
    VK_LOGIC_OP_XOR, // XOr
    VK_LOGIC_OP_OR, // Or
    VK_LOGIC_OP_NOR, // NOr
    VK_LOGIC_OP_EQUIVALENT, // Equivalent
    VK_LOGIC_OP_INVERT, // Invert
    VK_LOGIC_OP_OR_INVERTED, // OrReverse
    VK_LOGIC_OP_COPY_INVERTED, // CopyInverted
    VK_LOGIC_OP_OR_INVERTED, // OrInverted
    VK_LOGIC_OP_NAND, // Nand
    VK_LOGIC_OP_SET, // Set
};

static constexpr VkBlendFactor VkBlendFactors[] =
{
    VkBlendFactor(), // Unknown
	VK_BLEND_FACTOR_ZERO, // Zero
	VK_BLEND_FACTOR_ONE, // One
	VK_BLEND_FACTOR_SRC_COLOR, // SrcColor
	VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR, // OneMinusSrcColor
	VK_BLEND_FACTOR_DST_COLOR, // DestColor
	VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR, // OneMinusDestColor
	VK_BLEND_FACTOR_SRC_ALPHA, // SrcAlpha
	VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, // OneMinusSrcAlpha
	VK_BLEND_FACTOR_DST_ALPHA, // DestAlpha
	VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA, // OneMinusDestAlpha
	VK_BLEND_FACTOR_CONSTANT_COLOR, // ConstantColor
	VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR, // OneMinusConstantColor
	VK_BLEND_FACTOR_CONSTANT_ALPHA, // ConstantAlpha
	VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA, // OneMinusConstantAlpha
	VK_BLEND_FACTOR_SRC_ALPHA_SATURATE, // SrcAlphaSaturate
	VK_BLEND_FACTOR_SRC1_COLOR, // Src1Color
	VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR, // OneMinusSrc1Color
	VK_BLEND_FACTOR_SRC1_ALPHA, // Src1AlphaSRC1_ALPHA,
	VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA, // OneMinusSrc1Alpha
};

static constexpr VkBlendOp VkBlendOps[] =
{
    VkBlendOp(), // Unknown
	VK_BLEND_OP_ADD, // Add
    VK_BLEND_OP_SUBTRACT, // Subtract
    VK_BLEND_OP_REVERSE_SUBTRACT, // ReverseSubtract
    VK_BLEND_OP_MIN, // Min
    VK_BLEND_OP_MAX, // Max
};

static constexpr VkPipelineBindPoint VkPipelineBindPoints[] =
{
    VkPipelineBindPoint(),// Unknown
    VK_PIPELINE_BIND_POINT_GRAPHICS,// Graphics
    VK_PIPELINE_BIND_POINT_COMPUTE,// Compute
};

VkAttachmentLoadOp VkUtils::_vk_get_load_op(GPU::LoadOp load_op)
{
    return VkLoadOps[u32(load_op)];
}

VkAttachmentStoreOp VkUtils::_vk_get_store_op(GPU::StoreOp store_op)
{
    return VkStoreOps[u32(store_op)];
}

VkCompareOp VkUtils::_vk_get_compare_op(GPU::CompareOp compare_op)
{
    return VkCompareOps[u32(compare_op)];
}

VkPresentModeKHR VkUtils::_vk_get_present_mode(GPU::PresentMode present_mode)
{
    return VkPresentModes[u32(present_mode)];
}

VkMemoryPropertyFlags VkUtils::_vk_get_memory_properties(GPU::HeapUsage heap_usage)
{
    return VkMemoryProperties[u32(heap_usage)];
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
    return VkFilters[u32(filter)];
}

VkSamplerMipmapMode VkUtils::_vk_get_mipmap_mode(GPU::SamplerMipMapMode mipmap_mode)
{
    return VkMipMapModes[u32(mipmap_mode)];
}

VkSamplerAddressMode VkUtils::_vk_get_address_mode(GPU::SamplerAddressMode address_mode)
{
    return VkAddressModes[u32(address_mode)];
}

VkImageType VkUtils::_vk_get_image_type(GPU::TextureType texture_type)
{
    return VkImageTypes[u32(texture_type)];
}

VkImageViewType VkUtils::_vk_get_image_view_type(GPU::TextureViewType texture_view_type)
{
    return VkImageViewTypes[u32(texture_view_type)];
}

VkComponentSwizzle VkUtils::_vk_get_component_swizzle(GPU::ComponentSwizzle component_swizzle)
{
    return VkComponentSwizzles[u32(component_swizzle)];
}

VkFormat VkUtils::_vk_get_texture_format(GPU::TextureFormat texture_format)
{
    return VkTextureFormats[u32(texture_format)];
}

VkImageTiling VkUtils::_vk_get_tiling(GPU::TextureTiling texture_tiling)
{
    return VkImageTilings[u32(texture_tiling)];
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
    return VkDescriptorTypes[u32(descriptor_type)];
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

    if (HasValue(access_masks & GPU::AccessMasks::RenderAttachmentRead))
    {
        vk_access_masks |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    }
    if (HasValue(access_masks & GPU::AccessMasks::RenderAttachmentWrite))
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
    return VkImageLayouts[u32(texture_layout)];
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
    return VkVertexInputRates[u32(input_rate)];
}

VkFormat VkUtils::_vk_get_vertex_format(GPU::VertexFormat vertex_format)
{
    return VkVertexFormats[u32(vertex_format)];
}

VkPrimitiveTopology VkUtils::_vk_get_topology(GPU::PrimitiveTopology primitive_topology)
{
    return VkPrimitiveTopologies[u32(primitive_topology)];
}

VkPolygonMode VkUtils::_vk_get_polygon_mode(GPU::PolygonMode polygon_mode)
{
    return VkPolygonModes[u32(polygon_mode)];
}

VkCullModeFlags VkUtils::_vk_get_cull_mode(GPU::CullMode cull_mode)
{
    return VkCullModes[u32(cull_mode)];
}

VkFrontFace VkUtils::_vk_get_front_face(GPU::FrontFace front_face)
{
    return VkFrontFaces[u32(front_face)];
}

VkSampleCountFlagBits VkUtils::_vk_get_samples(GPU::SampleCount sample_count)
{
    return VkSampleCounts[u32(sample_count)];
}

VkLogicOp VkUtils::_vk_get_logic_op(GPU::LogicOp logic_op)
{
    return VkLogicOps[u32(logic_op)];
}

VkBlendFactor VkUtils::_vk_get_blend_factor(GPU::BlendFactor blend_factor)
{
    return VkBlendFactors[u32(blend_factor)];
}

VkBlendOp VkUtils::_vk_get_blend_op(GPU::BlendOp blend_op)
{
    return VkBlendOps[u32(blend_op)];
}

VkColorComponentFlags VkUtils::_vk_get_color_component_flags(GPU::ColorComponentFlags color_components)
{
    VkColorComponentFlags vk_flags = 0;
    
    if(HasValue(color_components & GPU::ColorComponentFlags::R))
    {
        vk_flags |= VK_COLOR_COMPONENT_R_BIT;
    }

    if(HasValue(color_components & GPU::ColorComponentFlags::G))
    {
        vk_flags |= VK_COLOR_COMPONENT_G_BIT;
    }

    if(HasValue(color_components & GPU::ColorComponentFlags::B))
    {
        vk_flags |= VK_COLOR_COMPONENT_B_BIT;
    }

    if(HasValue(color_components & GPU::ColorComponentFlags::A))
    {
        vk_flags |= VK_COLOR_COMPONENT_A_BIT;
    }

    return vk_flags;
}

VkPipelineBindPoint VkUtils::_vk_get_bind_point(GPU::PipelineBindPoint bind_point)
{
    return VkPipelineBindPoints[u32(bind_point)];
}