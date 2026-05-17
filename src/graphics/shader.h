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
    mem::Allocator* allocator;

    ShaderInfo shader_info;
    Slice<u8> shader_code;

    void init(mem::Allocator* _allocator, const ShaderInfo& info);
    void destroy();
};

}
