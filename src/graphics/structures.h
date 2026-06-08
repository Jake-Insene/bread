#pragma once
#include "gpu/gpu.h"
#include "graphics/shader.h"


namespace Graphics
{

struct PipelineLayout;

struct SamplerInfo
{
    GPU::Filter min_filter;
    GPU::Filter mag_filter;
    GPU::SamplerMipMapMode mipmap_mode;
    GPU::SamplerAddressMode address_mode_u;
    GPU::SamplerAddressMode address_mode_v;
    GPU::SamplerAddressMode address_mode_w;
    f32 mip_lod_bias;
    bool anisotropy_enable;
    f32 max_anisotropy;
    bool compare_enable;
    GPU::CompareOp compare_op;
    f32 min_lod;
    f32 max_lod;
};

struct TextureInfo
{
    GPU::TextureType type;
	GPU::TextureFormat format;
	Vector3U extent;
	u32 mip_levels;
	u32 array_levels;
	GPU::SampleCount sample_count;
	GPU::TextureTiling tiling;
	GPU::TextureUsage usage;
	GPU::TextureLayout initial_layout;
    GPU::TextureSubresourceRange subresource_range;
};

struct DescriptorSetInfo
{
    GPU::DeviceID gpu_device;
    GPU::DescriptorPoolID gpu_descriptor_pool;
    GPU::DescriptorSetLayoutID gpu_descriptor_set_layout;
};

union WriteInfo
{
    GPU::DescriptorBufferInfo buffer;
    GPU::DescriptorTextureInfo texture;
};

struct WriteArrayInfo
{
    Slice<GPU::DescriptorBufferInfo> buffers;
    Slice<GPU::DescriptorTextureInfo> textures;
};

struct DescriptorDeferredWrite
{
    u32 binding;
    GPU::DescriptorType type;
    WriteInfo write;
    WriteArrayInfo write_array;
};

struct DescriptorSetLayoutCreateInfo
{
	Slice<const GPU::DescriptorBinding> bindings;
};

struct PipelineLayoutInfo
{
	Slice<const GPU::ConstantBlock> constant_blocks;
	Slice<const DescriptorSetLayoutCreateInfo> set_layout_infos;
};

struct PipelineInfo
{
    GPU::PipelineBindPoint bind_point;
    Shader shader;
    GPU::VertexInput vertex_input;
	GPU::InputAssembly input_assembly;
    GPU::RasterizerState rasterizer_state;
	GPU::MultisampleState multisample_state;
    GPU::DepthStencilState depth_stencil_state;
    PipelineLayout* pipeline_layout;
    GPU::RenderingInfo rendering_info;
};

}