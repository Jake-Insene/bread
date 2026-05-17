#include "graphics/shader.h"

#include "io/file.h"


namespace Graphics
{

void Shader::init(mem::Allocator* _allocator, const ShaderInfo& info)
{
    allocator = _allocator;

    shader_info = info;
    shader_code = File::read_all(allocator, info.file_path);
}

void Shader::destroy()
{
    allocator->free(shader_code);
}

}
