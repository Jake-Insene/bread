#if !defined(BREAD_PROJECT_COMPILATION)
#error "this file should be included in the project not the engine"
#endif

#include "systems/system.h"
#include "render/render_device.h"
#include "render/2d/scene_renderer_2d.h"

SystemInfo _system_infos[] =
{
#if defined(BREAD_SYSTEM_RENDERDEVICE)
	RenderDevice::get_system_info(),
#endif
#if defined(BREAD_SYSTEM_SCENERENDERER2D)
	SceneRenderer2D::get_system_info(),
#endif
	IdentitySystem::get_system_info(),
};

Slice<SystemInfo> __get_requested_systems__()
{
	Slice<SystemInfo> requested_systems = _system_infos;

	return requested_systems.len == 1 ? Slice<SystemInfo>()
		: requested_systems.slice(requested_systems.len - 1);
}