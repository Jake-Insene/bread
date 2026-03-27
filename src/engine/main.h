#if !defined(BREAD_PROJECT_COMPILATION)
#error "this file should be included in the project not the engine"
#endif

#include "systems/system.h"
#include "render/render_device.h"
#include "render/pipeline/render_pipeline.h"
#include "resource/resource_manager.h"

SystemInfo _system_infos[] =
{
#if defined(BREAD_SYSTEM_RENDERDEVICE)
	RenderDevice::get_system_info(),
#endif
#if defined(BREAD_SYSTEM_RESOURCEMANAGER)
ResourceManager::get_system_info(),
#endif
#if defined(BREAD_SYSTEM_RENDERPIPELINE)
RenderPipeline::get_system_info(),
#endif
IdentitySystem::get_system_info(),
};

Slice<SystemInfo> __get_requested_systems__()
{
	Slice<SystemInfo> requested_systems = _system_infos;

	return requested_systems.len == 1 ? Slice<SystemInfo>()
		: requested_systems.slice(requested_systems.len - 1);
}