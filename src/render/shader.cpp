#include "render/shader.h"

#include "engine/engine.h"
#include "io/file.h"
#include "render/render_device.h"


Shader Shader::create(const ShaderInfo &info)
{
    Shader shader = {};
    
    RenderDevice* render_device = Engine::get_system_manager()->get_system<RenderDevice>();

    shader.init(render_device->allocator, info);
    return shader;
}

void Shader::init(const mem::Allocator _allocator, const ShaderInfo& info)
{
    allocator = _allocator;

    shader_info = info;
    shader_code = File::read_all(allocator, info.file_path);
}

void Shader::destroy()
{
    allocator.free(shader_code);
}
