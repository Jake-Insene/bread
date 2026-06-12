#if !defined(BREAD_PROJECT_COMPILATION)
#error "this file should be included in the project not the engine"
#endif

#include "systems/system.h"

static inline SystemInfo _system_infos[] =
{
	IdentitySystem::get_system_info(),
};

Slice<SystemInfo> __get_requested_systems__()
{
	Slice requested_systems = _system_infos;

	return requested_systems.len == 1 ? Slice<SystemInfo>()
		: requested_systems.slice(requested_systems.len - 1);
}