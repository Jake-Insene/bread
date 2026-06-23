#include "graphics/shader.h"

#include "io/file.h"


namespace Graphics
{

void Shader::init(Mem::Allocator* allocator, const ShaderInfo& info)
{
    data.allocator = allocator;

    data.shader_info = info;
    data.shader_code = IO::File::read_all(data.allocator, info.file_path);

    data.shader_stages[0] = GPU::ShaderStageInfo
    {
        .stage = GPU::ShaderStage::Vertex,
        .code = data.shader_code,
        .name = info.vertex_name,
    };

    data.shader_stages[1] = GPU::ShaderStageInfo
    {
        .stage = GPU::ShaderStage::Fragment,
        .code = data.shader_code,
        .name = info.fragment_name,
    };
}

void Shader::destroy()
{
    data.allocator->free(data.shader_code);
}

}
