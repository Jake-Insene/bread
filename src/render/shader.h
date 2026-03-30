#pragma once
#include "collections/string_view.h"
#include "gpu/gpu.h"


struct ShaderInfo
{
    StringView file_path;

    StringView vertex_name;
    StringView fragment_name;
};

struct Shader
{
    mem::Allocator allocator;

    ShaderInfo shader_info;
    Slice<u8> shader_code;

    static Shader create(const ShaderInfo& info);

    void init(const mem::Allocator _allocator, const ShaderInfo& info);
    void destroy();
};
