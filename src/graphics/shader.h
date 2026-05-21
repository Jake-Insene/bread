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
    Mem::Allocator* allocator;

    ShaderInfo shader_info;
    Slice<u8> shader_code;

    void init(Mem::Allocator* _allocator, const ShaderInfo& info);
    void destroy();
};

}
