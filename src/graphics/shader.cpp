#include "graphics/shader.h"

#include "io/file.h"


namespace Graphics
{

void Shader::init(Mem::Allocator* _allocator, const ShaderInfo& info)
{
    allocator = _allocator;

    shader_info = info;
    shader_code = IO::File::read_all(allocator, info.file_path);

    shader_stages[0] = GPU::ShaderStageInfo
    {
        .stage = GPU::ShaderStage::Vertex,
        .code = shader_code,
        .name = info.vertex_name,
    };

    shader_stages[1] = GPU::ShaderStageInfo
    {
        .stage = GPU::ShaderStage::Fragment,
        .code = shader_code,
        .name = info.fragment_name,
    };
}

void Shader::destroy()
{
    allocator->free(shader_code);
}

}
