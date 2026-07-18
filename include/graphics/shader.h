#pragma once
#include "collections/string_view.h"
#include "gpu/gpu.h"


namespace Graphics
{

struct ShaderInfo
{
    StringView file_path;

    StringView vertex_name;
    StringView fragment_name;
};

struct Shader
{
    struct InternalData
    {
        Mem::Allocator* allocator;

        ShaderInfo shader_info;
        Slice<u8> shader_code;
        GPU::ShaderStageInfo shader_stages[2];
    } data;

    void init(Mem::Allocator* allocator, const ShaderInfo& info);
    void destroy();

    Slice<const GPU::ShaderStageInfo> get_stages() const { return data.shader_stages; }
};

}
