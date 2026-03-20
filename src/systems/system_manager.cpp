#include "systems/system_manager.h"


void SystemManager::init(const mem::Allocator& _allocator)
{
    allocator = _allocator;

    systems = Array<SystemInstance>::with_allocator(allocator);
}

void SystemManager::destroy()
{
    systems.destroy();
}

void SystemManager::allocate_systems(const Slice<SystemInfo>& requested_systems)
{
    for(const SystemInfo& system_info : requested_systems)
    {
        Slice<u8> instance_bytes = allocator.alloc(system_info.size_in_bytes, 16);

        SystemInstance instance =
        {
            .info = system_info,
            .instance = reinterpret_cast<Opaque*>(instance_bytes.ptr()),
        };

        (void)systems.add(instance);
    }

    _initialize_systems_instance();
}

void SystemManager::deallocate_systems()
{
    _shutdown_systems_instance();
    
    for(SystemInstance& instance : systems.iter())
    {
        allocator.free(mem::to_bytes(Slice(instance.instance, 1)));
    }

    systems.clear();
}

void SystemManager::_initialize_systems_instance()
{
    for(SystemInstance& instance : systems.iter())
    {
        SystemInitializeInfo info =
        {
            .allocator = allocator,
        };
        instance.info.runtime.initialize.call(instance.instance, info);
    }
}

void SystemManager::_shutdown_systems_instance()
{
    for(SystemInstance& instance : systems.iter())
    {
        instance.info.runtime.shutdown.call(instance.instance);
    }
}

Opaque* SystemManager::_get_system(const SystemInfo& info)
{
    for(SystemInstance& instance : systems.iter())
    {
        if(instance.info.name.equals(info.name))
        {
            return instance.instance;
        }
    }

    return nullptr;
}
